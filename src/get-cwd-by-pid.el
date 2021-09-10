;;; get-cwd-by-pid.el --- Description -*- lexical-binding: t; -*-
;;
;; Copyright (C) 2021 Chris Schwaab
;;
;; Author: Chris Schwaab <https://github.com/christopherschwaab>
;; Maintainer: Chris Schwaab <chris.schwaab@gravitysketch.com>
;; Created: September 10, 2021
;; Modified: September 10, 2021
;; Version: 0.0.1
;; Keywords: abbrev bib c calendar comm convenience data docs emulations extensions faces files frames games hardware help hypermedia i18n internal languages lisp local maint mail matching mouse multimedia news outlines processes terminals tex tools unix vc wp
;; Homepage: https://github.com/christopherschwaab/emacs-get-cwd-by-pid
;; Package-Requires: ((emacs "24.3"))
;;
;; This file is not part of GNU Emacs.
;;
;;; Commentary:
;;
;;  Description
;;
;;; Code:

(unless (require 'emacs-get-cwd-by-pid nil t)
  (error "Failed to load the get-cwd-by-pid dynamic module which is required on windows!"))

(provide 'get-cwd-by-pid)
;;; get-cwd-by-pid.el ends here
