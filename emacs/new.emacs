;; Cleaned and improved .emacs configuration
;; ------------------------------------------------------------
;; Package System Setup
;; ------------------------------------------------------------
(require 'package)
(add-to-list 'package-archives '("melpa" . "https://melpa.org/packages/") t)
(package-initialize)

;; Bootstrap use-package
(unless (package-installed-p 'use-package)
  (package-refresh-contents)
  (package-install 'use-package))

(require 'use-package)
(setq use-package-always-ensure t)

;; ------------------------------------------------------------
;; UI / Quality of Life
;; ------------------------------------------------------------
(setq inhibit-startup-screen t)
(global-display-line-numbers-mode 1)
(menu-bar-mode -1)
(tool-bar-mode -1)
(scroll-bar-mode -1)
(setq-default line-spacing 0.4)

;; Centering cursor
(use-package centered-cursor-mode
  :hook ((text-mode prog-mode) . centered-cursor-mode))

;; Custom theme (fish)
(load-file "~/.emacs.d/fish-theme.el")
(enable-theme 'fish)

;; Autosave directory
(unless (file-exists-p "~/autosave/")
  (make-directory "~/autosave/"))
(setq auto-save-file-name-transforms '((".*" "~/autosave/" t)))

;; ------------------------------------------------------------
;; LSP + Completion
;; ------------------------------------------------------------
(use-package company
  :init (global-company-mode))

(use-package lsp-mode
  :hook ((c++-mode . lsp)
         (c-mode   . lsp))
  :commands lsp
  :config
  (define-key lsp-mode-map (kbd "C-x C-a") #'lsp-signature-help)
  (define-key lsp-mode-map (kbd "C-x C-d") #'lsp-describe-session))

(use-package lsp-ui
  :hook (lsp-mode . lsp-ui-mode))

;; ------------------------------------------------------------
;; TypeScript / TSX
;; ------------------------------------------------------------

;; Major mode for .ts / .tsx
(use-package typescript-mode
  :mode ("\\.ts\\'" "\\.tsx\\'")
  :init
  (setq typescript-indent-level 2))

;; Optional: treat TSX as web-mode (better JSX/TSX editing)
;; If you prefer to keep .tsx in typescript-mode, remove this.
(use-package web-mode
  :mode ("\\.tsx\\'" "\\.jsx\\'")
  :init
  (setq web-mode-markup-indent-offset 2
        web-mode-code-indent-offset 2
        web-mode-css-indent-offset 2))

;; LSP integration for TypeScript/JavaScript
;; Requires: npm i -g typescript typescript-language-server
(use-package lsp-mode
  :hook ((typescript-mode . lsp)
         (web-mode        . (lambda ()
                              (when (string-match-p "\\.[jt]sx\\'" (or buffer-file-name ""))
                                (lsp))))
         (js-mode         . lsp)
         (js2-mode        . lsp))
  :config
  ;; Prefer LSP completion (company-capf)
  (setq lsp-completion-provider :capf)

  ;; TypeScript-specific LSP settings
  (setq lsp-clients-typescript-preferences
        '(:includeCompletionsForModuleExports t
          :includeCompletionsWithInsertText t))

  ;; If you don't want LSP formatting (because you use Prettier), disable it:
  ;; (setq lsp-enable-on-type-formatting nil)
  ;; (setq lsp-enable-indentation nil)
  ;; (setq lsp-enable-formatting nil)
  )

;; Prettier formatting
;; Requires: npm i -g prettier
;; If you use project-local prettier, this still works via node_modules/.bin if in PATH.
(use-package prettier-js
  :hook ((typescript-mode . prettier-js-mode)
         (web-mode        . prettier-js-mode)
         (js-mode         . prettier-js-mode)))

;; Linting (optional but recommended)
;; Requires: npm i -g eslint (or project-local)
(use-package flycheck
  :init (global-flycheck-mode))

(use-package lsp-ui
  :after lsp-mode
  :config
  ;; Nice defaults for TS/JS (optional)
  (setq lsp-ui-doc-enable t
        lsp-ui-sideline-enable t
        lsp-ui-sideline-show-code-actions t))


;; ------------------------------------------------------------
;; LaTeX / AUCTeX / PDF Tools
;; ------------------------------------------------------------
(use-package auctex
  :defer t)

(use-package tex
  :ensure auctex
  :config
  ;; Add LatexMk
  (add-to-list 'TeX-command-list
               '("LatexMk" "latexmk -pdf -interaction=nonstopmode %s"
                 TeX-run-TeX nil t :help "Run latexmk"))

  (setq TeX-command-default "LatexMk"
        TeX-source-correlate-mode t
        TeX-source-correlate-method 'synctex
        TeX-save-query nil
        TeX-master nil)

  ;; Auto-refresh PDF buffer
  (add-hook 'TeX-after-compilation-finished-functions
            #'TeX-revert-document-buffer)

  ;; PDF Tools viewer
  (setq TeX-view-program-selection '((output-pdf "PDF Tools")))
  (setq TeX-view-program-list '(("PDF Tools" TeX-pdf-tools-sync-view))))

(use-package pdf-tools
  :config
  (pdf-tools-install))


(add-hook 'LaTeX-mode-hook
          (lambda ()
            (add-hook 'after-save-hook
                      #'TeX-command-master nil t)))

;; ------------------------------------------------------------
;; Org-Mode (uses built-in version)
;; ------------------------------------------------------------
(setq org-file-apps
      '((auto-mode . emacs)
        ("\\.pdf\\'" . "evince %s")
        ("\\.x?html?\\'" . "firefox %s")))

;; ------------------------------------------------------------
;; Keybindings
;; ------------------------------------------------------------
(global-set-key (kbd "C-x t") #'lsp-clangd-find-other-file)
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(package-selected-packages nil))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )
