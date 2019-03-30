" What: Complete sentence using the surrent files and mostFeqLines.txt
" Usage: :source the plugin and then use :SSC to activate autocomplete and
" :USSC to deactivate. press C-x C-o for auto complete.
" How:  Get sentence completions using perl from current  file and a file with
" frequent sentences.

set completeopt+=menuone

function! s:PrintHelloWorld()
	echo "Hello World"
endfunction

command! PHW call s:PrintHelloWorld()

function! s:SetSentenceComplete()
	let s:originalOmniFunction=&omnifunc 
	set omnifunc=HariCompleteSentence
endfunction

command! SSC call s:SetSentenceComplete()

function! s:UnsetSentenceComplete()
	let &omnifunc = s:originalOmniFunction
endfunction

command! USSC call s:UnsetSentenceComplete()

function!    HariCompleteSentence(findstart, base)
    " see :help  complete-functions.
    " This function is called twice upon pressing <C-x C-o>
    " First time it returns the character from which the autocompletion starts
    if a:findstart
        let s:line = getline('.')
        let s:wordStart = col('.') - 1
	" match till the last space. To make sure the we don't auto complete
	" from the previous words
        while s:wordStart > 0 && s:line[s:wordStart - 1] !~ '\s'
            let s:wordStart -= 1
        endwhile
        return s:wordStart

    else
        " return list of possible sentences. These matching words contain
	" a:base
        let a:sentence_regex = a:base
	echom "Base is : " . string(a:base)
        " in regex trim spaces
	" \v to indicate all the following characters are magical.
        let a:sentence_regex = substitute(a:sentence_regex,'\v^\s*(.{-})\s*$','\1','')

	echom a:sentence_regex
	let s:current_script_path = string(fnamemodify(resolve(expand('<sfile>:p')), ':h'))
	let s:most_frequent_lines_file_path = s:current_script_path . "/most_freq_lines.txt"

        " in regex change punctuation as dot.
        let a:sentence_regex = substitute(a:sentence_regex,'\W','.','g')
        " grep using perl
        let s:cmd='perl -ne '' '
            \.'chomp;'
            \.'next if m/^[;#]/;'
            \.'if( /('.a:sentence_regex.'.{1,30})/io ){'
            \.'   print qq/$1;;/ '
            \.'}'
            \. ' '' '
        let s:current_file_cmd = s:cmd . expand("%:p")
	let s:most_freq_file_cmd = s:cmd . s:most_frequent_lines_file_path

        echom s:current_file_cmd
        let   s:rawOutput = system(s:current_file_cmd)
        let   s:current_file_matches = split(s:rawOutput, ';;')

        echom s:most_freq_file_cmd
        let   s:rawOutput = system(s:most_freq_file_cmd)
        let   s:most_frequent_file_matches = split(s:rawOutput, ';;')

	let s:listing = extend(s:current_file_matches, s:most_frequent_file_matches)
		
        return s:listing
    endif
endfunction
