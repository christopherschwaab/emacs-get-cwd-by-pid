This allows emacs to keep track of the current directory a shell is in, such as
shell mode. A native module inspects the shell process' process environment
block (PEB) to extract the current directory. This means you don't need to print
the current directory in your prompt, and you don't have to worry about emacs
losing track of your current directory any time an unrecognized command or batch
script changes it.

To set this up with straight, try

    (use-package shell-intrusive-dirtrack-mode
      :straight (shell-intrusive-dirtrack-mode
                 :type git
                 :host github
                 :repo "christopherschwaab/emacs-get-cwd-by-pid")
      :hook (shell-mode . shell-intrusive-dirtrack-mode))

and with doom add the following to your `packages.el` file

    (package! shell-intrusive-dirtrack-mode
      :recipe (:host github :repo "christopherschwaab/emacs-get-cwd-by-pid"))

and then in `config.el`

    (use-package shell-intrusive-dirtrack-mode
      :hook (shell-mode . shell-intrusive-dirtrack-mode))
