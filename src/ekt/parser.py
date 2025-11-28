from .exceptions import *

template_start = "![["
template_end = "]]"

class ParsedOutput:
    def __init__(self, str_parts, variables):
        self.str_parts = str_parts
        self.variables = variables

class StrPart:
    def __init__(self, start, end, template):
        self.start = start
        self.end = end
        self.template = template

def parse(input):
    str_parts = [
        StrPart(0, len(input), False)
    ]
    variables = []
    template_start_idx = input.find(template_start)
    while template_start_idx != -1:
        template_start_idx += str_parts[-1].start
        str_parts[-1].end = template_start_idx
        first_char_idx = template_start_idx + len(template_start)
        last_char_idx = -1
        str_len = len(input) - first_char_idx
        for i,c in enumerate(input[first_char_idx:]):
            chars_left = str_len - i - 1
            j = i + first_char_idx
            if chars_left >= 2 and input[j:j+3] == template_start:
                print("TODO support recursion?")
                break
            elif chars_left >= 1 and input[j:j+2] == template_end:
                last_char_idx = j
                str_parts.append(StrPart(first_char_idx, last_char_idx, True))
                str_parts.append(StrPart(j+2, len(input), False))
                variables.append(input[first_char_idx:last_char_idx].lower())
                break
        
        if last_char_idx == -1:
            raise PoorlyFormedTemplateVariable()
        
        template_start_idx = input[str_parts[-1].start:].find(template_start)
 
    return ParsedOutput(str_parts, variables)
