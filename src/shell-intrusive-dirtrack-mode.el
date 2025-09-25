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
(require 'url)

(defvar shell-intrusive-dirtrack-version "v0.1.0"
  "Version of the dynamic module to download.")

(defvar shell-intrusive-dirtrack-module-name "libemacs-get-cwd-by-pid"
  "Name of the dynamic module for shell-intrusive-dirtrack-mode.")

(defvar shell-intrusive-dirtrack-install-directory
  (file-name-directory load-file-name)
  "Directory where the dynamic module should be installed.")

(defvar shell-intrusive-dirtrack-download-url
  (format "https://github.com/christopherschwaab/emacs-get-cwd-by-pid/releases/download/%s/libemacs-get-cwd-by-pid.dll"
          shell-intrusive-dirtrack-version)
  "URL to download the dynamic module from.")

(defun shell-intrusive-dirtrack--module-path ()
  "Return the full path to the dynamic module."
  (when (eq system-type 'windows-nt)
    (expand-file-name (concat shell-intrusive-dirtrack-module-name ".dll")
                      shell-intrusive-dirtrack-install-directory)))

(defun shell-intrusive-dirtrack--download-module ()
  "Download the dynamic module from GitHub releases."
  (when (eq system-type 'windows-nt)
    (let ((module-path (shell-intrusive-dirtrack--module-path))
          (url shell-intrusive-dirtrack-download-url))
      (message "Downloading %s to %s..." url module-path)
      (url-copy-file url module-path t)
      (message "Downloaded %s successfully" module-path))))

(defun shell-intrusive-dirtrack--ensure-module ()
  "Ensure the dynamic module is available, offering to download if missing."
  (when (eq system-type 'windows-nt)
    (let ((module-path (shell-intrusive-dirtrack--module-path)))
      (unless (file-exists-p module-path)
        (when (y-or-n-p (format "Dynamic module %s not found. Download it automatically? " module-path))
          (condition-case err
              (shell-intrusive-dirtrack--download-module)
            (error (user-error "Failed to download module: %s" (error-message-string err))))))
      (unless (file-exists-p module-path)
        (user-error "Dynamic module %s not found. Please build or download it manually" module-path)))))

(shell-intrusive-dirtrack--ensure-module)

(when (eq system-type 'windows-nt)
  (unless (require 'libemacs-get-cwd-by-pid nil t)
    (error "Failed to load the get-cwd-by-pid dynamic module which is required on windows!")))

(defun shell-intrusive-dirtrack (str)
  "Track shell directory by inspecting the CWD of the shell process by PID."
  (prog1 str
    (when (string-match comint-prompt-regexp str)
      (let ((pid (process-id (get-buffer-process (current-buffer)))))
        (cond
         ((or (eq system-type 'windows-nt) (eq system-type 'ms-dos) (eq system-type 'cygwin))
          (let ((directory (string-trim-right
                           (shell-command-to-string
                            (concat "cygpath -m '" (get-cwd-by-pid pid) "'")))))
            (when (file-directory-p directory)
              (cd directory))))
         ((or (eq system-type 'gnu/linux) (eq system-type 'linux))
          (let ((directory (file-symlink-p (format "/proc/%s/cwd" pid))))
            (when (file-directory-p directory)
              (cd directory))))
         (t (error "The system-type '%s' is not supported" system-type)))))))

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

;;;###autoload
(defun shell-intrusive-dirtrack-install-module ()
  "Interactively install or reinstall the dynamic module."
  (interactive)
  (if (eq system-type 'windows-nt)
      (let ((module-path (shell-intrusive-dirtrack--module-path)))
        (when (or (not (file-exists-p module-path))
                  (y-or-n-p (format "Module already exists at %s. Reinstall? " module-path)))
          (condition-case err
              (shell-intrusive-dirtrack--download-module)
            (error (user-error "Failed to download module: %s" (error-message-string err))))
          (message "Module installed successfully. You may need to restart Emacs.")))
    (message "Module installation is only needed on Windows. This system uses /proc for directory tracking.")))

(provide 'shell-intrusive-dirtrack-mode)
;;; shell-intrusive-dirtrack-mode.el ends here
