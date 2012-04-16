;;; mu4e-main.el -- part of mu4e, the mu mail user agent
;;
;; Copyright (C) 2011-2012 Dirk-Jan C. Binnema

;; Author: Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
;; Maintainer: Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>

;; This file is not part of GNU Emacs.
;;
;; GNU Emacs is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Code:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(require 'mu4e-utils)    ;; utility functions

(defconst mu4e-main-buffer-name "*mu4e-main*"
  "*internal* Name of the mu4e main view buffer.")

(defvar mu4e-main-mode-map
  (let ((map (make-sparse-keymap)))

    (define-key map "b" 'mu4e-search-bookmark)
    (define-key map "B" 'mu4e-search-bookmark-edit-first)

    (define-key map "s" 'mu4e-search)
    (define-key map "q" 'mu4e-quit)
    (define-key map "j" 'mu4e-jump-to-maildir)
    (define-key map "C" 'mu4e-compose-new)

    (define-key map "m" 'mu4e-toggle-mail-sending-mode)
    (define-key map "f" 'smtpmail-send-queued-mail)
    (define-key map "U" 'mu4e-update-mail-show-window)

    (define-key map "$" 'mu4e-show-log)
    (define-key map "H" 'mu4e-display-manual)
    map)

  "Keymap for the *mu4e-main* buffer.")
(fset 'mu4e-main-mode-map mu4e-main-mode-map)

(define-derived-mode mu4e-main-mode special-mode "mu4e:main"
  "Major mode for the mu4e main screen.
\\{mu4e-main-mode-map}."
  (use-local-map mu4e-main-mode-map)
  (setq
    truncate-lines t
    overwrite-mode 'overwrite-mode-binary))


(defun mu4e-action-str (str &optional func-or-shortcut)
  "Highlight the first occurence of [..] in STR. If
FUNC-OR-SHORTCUT is non-nil and if it is a function, call it when
STR is clicked (using RET or mouse-2); if FUNC-OR-SHORTCUT is a
string, execute the corresponding keyboard action when it is
clicked."
  (let ((newstr
	  (replace-regexp-in-string
	    "\\[\\(\\w+\\)\\]"
	    (lambda(m)
	      (format "[%s]"
		(propertize (match-string 1 str) 'face 'mu4e-highlight-face))) str))
	 (map (make-sparse-keymap))
	 (func (if (functionp func-or-shortcut)
		 func-or-shortcut
		 (if (stringp func-or-shortcut)
		   (lexical-let ((macro func-or-shortcut))
		     (lambda()(interactive)
		       (execute-kbd-macro macro)))))))
    (define-key map [mouse-2] func)
    (define-key map (kbd "RET") func)
    (put-text-property 0 (length newstr) 'keymap map newstr)
    (put-text-property (string-match "\\w" newstr)
      (- (length newstr) 1) 'mouse-face 'highlight newstr)
    newstr))

(defun mu4e-main-view()
  "Show the mu4e main view."
  (let ((buf (get-buffer-create mu4e-main-buffer-name))
	 (inhibit-read-only t))
    (with-current-buffer buf
      (erase-buffer)
      (insert
	"* "
	(propertize "mu4e - mu for emacs version " 'face 'mu4e-title-face)
	(propertize  mu4e-mu-version 'face 'mu4e-view-header-key-face)
	"\n\n"
	(propertize "  Basics\n\n" 'face 'mu4e-title-face)
	(mu4e-action-str "\t* [j]ump to some maildir\n" 'mu4e-jump-to-maildir)
	(mu4e-action-str "\t* enter a [s]earch query\n" 'mu4e-search)
	(mu4e-action-str "\t* [C]ompose a new message\n" 'mu4e-compose-new)
	"\n"
	(propertize "  Bookmarks\n\n" 'face 'mu4e-title-face)
	;; TODO: it's a bit uncool to hard-code the "b" shortcut...
	(mapconcat
	  (lambda (bm)
	    (let* ((query (nth 0 bm)) (title (nth 1 bm)) (key (nth 2 bm)))
	      (mu4e-action-str
		(concat "\t* [b" (make-string 1 key) "] " title)
		(concat "b" (make-string 1 key)))))
	  mu4e-bookmarks "\n")
	"\n"
	(propertize "  Misc\n\n" 'face 'mu4e-title-face)

	(mu4e-action-str "\t* [U]pdate email & database\n"
	  'mu4e-update-mail-show-window)

	;; show the queue functions if `smtpmail-queue-dir' is defined
	(if (file-directory-p smtpmail-queue-dir)
	  (concat
	    (mu4e-action-str "\t* toggle [m]ail sending mode "
	      'mu4e-toggle-mail-sending-mode)
	    "(" (propertize (if smtpmail-queue-mail "queued" "direct")
		  'face 'mu4e-view-header-key-face) ")\n"
	    (mu4e-action-str "\t* [f]lush queued mail\n"
	      'smtpmail-send-queued-mail))
	  "")
	"\n"

	(mu4e-action-str "\t* [H]elp\n" 'mu4e-display-manual)
	(mu4e-action-str "\t* [q]uit\n" 'mu4e-quit))
      (mu4e-main-mode)
      (switch-to-buffer buf)
      (delete-other-windows))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Interactive functions

(defconst mu4e-update-buffer-name "*mu4e-update*"
  "*internal* Name of the buffer for message retrieval / database
  updating.")

(defun mu4e-update-mail-show-window ()
  "Try to retrieve mail (using the user-provided shell command),
and update the database afterwards, and show the progress in a
split-window."
  (interactive)
  (unless mu4e-get-mail-command
    (error "`mu4e-get-mail-command' is not defined"))
  (let ((buf (get-buffer-create mu4e-update-buffer-name))
	 (win
	   (split-window (selected-window)
	     (- (window-height (selected-window)) 8))))
    (with-selected-window win
      (switch-to-buffer buf)
      (set-window-dedicated-p win t)
      (erase-buffer)
      (insert "\n") ;; FIXME -- needed so output starts
      (mu4e-update-mail buf))))


(defun mu4e-toggle-mail-sending-mode ()
  "Toggle sending mail mode, either queued or direct."
  (interactive)
  (unless (file-directory-p smtpmail-queue-dir)
    (error "`smtp-queue-dir' does not exist"))
  (setq smtpmail-queue-mail (not smtpmail-queue-mail))
  (message
    (concat "Outgoing mail will now be "
      (if smtpmail-queue-mail "queued" "sent directly")))
  (mu4e-main-view))

(provide 'mu4e-main)
