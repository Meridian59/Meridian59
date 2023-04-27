; Mode for editing Blakod
; Author: Andrew Kirmse
;
; Inspired by http://www.emacswiki.org/emacs/ModeTutorial
;
; Usage:
; (require 'blakod-mode)
;
; Known problems:
; - Doesn't indent on continuation lines ending in backslash
; - Doesn't indent on unbalanced parentheses, e.g. inside function calls.
; - Inserts tabs instead of spaces

(defvar blakod-mode-hook nil)

(defvar blakod-mode-map
  (let ((map (make-keymap)))
    (define-key map "\C-j" 'newline-and-indent)
    map)
  "Keymap for Blakod major mode")

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.kod\\'" . blakod-mode))
(add-to-list 'auto-mode-alist '("\\.khd\\'" . blakod-mode))

;
; Font lock
;

(defconst blakod-font-lock-keywords-1
  (list
   '("\\<\\(return\\|include\\|constants\\|resources\\|classvars\\|properties\\|messages\\|propagate\\|if\\|else\\|local\\|and\\|or\\|mod\\|not\\|AND\\|OR\\|MOD\\|NOT\\|while\\|for\\|in\\|break\\|continue\\|is\\)\\>" . font-lock-keyword-face)
   '("\\<\\(Send\\|Create\\|Cons\\|First\\|Rest\\|Length\\|List\\|Nth\\|SetFirst\\|SetNth\\|DelListElem\\|FindListElem\\|Random\\|AddPacket\\|SendPacket\\|SendCopyPacket\\|ClearPacket\\|Debug\\|GetInactiveTime\\|DumpStack\\|StringEqual\\|StringContain\\|StringSubstitute\\|StringLength\\|StringConsistsOf\\|CreateTimer\\|DeleteTimer\\|IsList\\|IsClass\\|RoomData\\|LoadRoom\\|GetClass\\|GetTime\\|CanMoveInRoom\\|CanMoveInRoomFine\\|SetResource\\|Post\\|Abs\\|Sqrt\\|ParseString\\|CreateTable\\|AddTableEntry\\|GetTableEntry\\|DeleteTableEntry\\|DeleteTable\\|Bound\\|GetTimeRemaining\\|SetString\\|AppendTempString\\|ClearTempString\\|GetTempString\\|CreateString\\|IsObject\\|RecycleUser\\|MinigameNumberToString\\|MinigameStringToNumber\\)\\>" . font-lock-builtin-face)
   '("\\<\\(\\$\\|-?[0-9]+\\|0x[0-9a-fA-f]+\\)\\>" . font-lock-constant-face)
   '("\\('\\w*'\\)" . font-lock-variable-name-face))
  "Minimal highlighting expressions for Blakod mode")

(defvar blakod-font-lock-keywords blakod-font-lock-keywords-1
  "Default highlighting expressions for Blakod mode")

;
; Indenting
;

; - Top of the file indents 0
; - Close brace outdents
; - Section keywords (properties, classvars, end, etc.) indent 0
; - Searching backwards, indent same as previous close brace
; - Searching backwards, indent on section keywords or open brace

(setq default-tab-width 3)
(setq indent-tabs-mode nil)

(defun blakod-indent-line ()
  "Indent current line as Blakod"
  (interactive)
  (beginning-of-line)
  (if (or (bobp)  ; Beginning of buffer
			 (looking-at "^[ \t]*\\(constants\\|classvars\\|resources\\|properties\\|messages\\|end\\)"))  ; Section keyword
      (indent-line-to 0)
    (let ((not-indented t) cur-indent)
      (if (looking-at "^[ \t]*}") ; Outdent on close brace
	  (progn
	    (save-excursion
	      (forward-line -1)
	      (setq cur-indent (- (current-indentation) default-tab-width)))
	    (if (< cur-indent 0)
		(setq cur-indent 0)))
	(save-excursion 
	  (while not-indented
		 (forward-line -1)
		 (if (or (looking-at "^[ \t]*}")  ; Found a close brace before this line
					(looking-at "^[ \t]\\\""))  ; Align with string on previous line
			  (progn
				 (setq cur-indent (current-indentation))
				 (setq not-indented nil)) 
			(if (or (looking-at "^[ \t]*\\(constants\\|classvars\\|resources\\|properties\\|messages\\|{\\)")  ; Section keyword
					  (looking-at "\\\\[ \t]*$"))  ; Line ending in a continuation backslash
				 (progn
					(setq cur-indent (+ (current-indentation) default-tab-width))
					(setq not-indented nil))
			  (if (bobp)
					(setq not-indented nil)))))))
      
      (if cur-indent
          (indent-line-to cur-indent)
        ; If we didn't see an indentation hint, then allow no indentation
        (indent-line-to 0)))))

;
; Syntax table
;

(defvar blakod-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?_ "w" st)  ; Underscore is a word char
    (modify-syntax-entry ?% "<" st)  ; % starts a comment
    (modify-syntax-entry ?\n ">" st)  ; newline ends a comment
    st)
  "Syntax table for blakod-mode")

;
; Mode creation
;

(defun blakod-mode ()
  "Major mode for editing Blakod files"
  (interactive)
  (kill-all-local-variables)
  (set-syntax-table blakod-mode-syntax-table)
  (use-local-map blakod-mode-map)
  (set (make-local-variable 'font-lock-defaults) '(blakod-font-lock-keywords))
  (set (make-local-variable 'indent-line-function) 'blakod-indent-line) 
  (setq major-mode 'blakod-mode)
  (setq mode-name "Blakod")
  (run-hooks 'blakod-mode-hook))

(provide 'blakod-mode)