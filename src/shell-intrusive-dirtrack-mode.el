;;; shell-intrusive-dirtrack-mode.el --- Description -*- lexical-binding: t; -*-
;;
;; Copyright (C) 2021 Chris Schwaab
;;
;; Author: Chris Schwaab <https://github.com/christopherschwaab>
;; Maintainer: Chris Schwaab <chris.schwaab@gravitysketch.com>
;; Created: September 10, 2021
;; Modified: September 10, 2021
;; Version: 0.0.1
;; Keywords: processes terminals tools
;; Homepage: https://github.com/christopherschwaab/emacs-get-cwd-by-pid
;; Package-Requires: ((emacs "25.0"))
;;
;; This file is not part of GNU Emacs.
;;
;;; Commentary:
;;
;;  Description
;;
;;; Code:

(require 'subr-x)

(unless (require 'libemacs-get-cwd-by-pid nil t)
  (error "Failed to load the get-cwd-by-pid dynamic module which is required on windows!"))

(cond
 ((or (eq system-type 'windows-nt) (eq system-type 'ms-dos) (eq system-type 'cygwin))
  (defun shell-intrusive-dirtrack (str)
    (prog1 str
      (when (string-match comint-prompt-regexp str)
        (let* ((pid (process-id (get-buffer-process (current-buffer))))
               ; TODO cygpath seems to put junk on the end (probably should do optional / processing in get-cwd-by-pid anyway)?
               (directory (string-trim-right (shell-command-to-string (concat "cygpath -m '" (get-cwd-by-pid pid) "'")))))
          (when (file-directory-p directory)
            (cd directory)))))))
 ((or (eq system-type 'gnu/linux) (eq system-type 'linux) (eq system-type 'gnu/kfreebsd) (eq system-type 'berkeley-unix))
  (defun shell-intrusive-dirtrack (str)
    (prog1 str
      (when (string-match comint-prompt-regexp str)
        (let ((directory (file-symlink-p
                          (format "/proc/%s/cwd"
                                  (process-id
                                   (get-buffer-process
                                    (current-buffer)))))))
          (when (file-directory-p directory)
            (cd directory)))))))
 (t (error (format "The system-type '%s' is not supported." system-type))))

;;;###autoload
(define-minor-mode shell-intrusive-dirtrack-mode
  "Track shell directory by inspecting the CWD of the shell process by PID.

The current working directory is discovered by PEB inspection on windows and on
by procfs on linux."
  :init-value nil
  :lighter nil
  :keymap nil
  (cond (shell-intrusive-dirtrack-mode
         (when (bound-and-true-p shell-dirtrack-mode)
           (shell-dirtrack-mode 0))
         (when (bound-and-true-p dirtrack-mode)
           (dirtrack-mode 0))
         (add-hook 'comint-preoutput-filter-functions
                   'shell-intrusive-dirtrack nil t))
        (t
         (remove-hook 'comint-preoutput-filter-functions
                      'shell-intrusive-dirtrack t))))

(provide 'shell-intrusive-dirtrack-mode)
;;; shell-intrusive-dirtrack-mode.el ends here
