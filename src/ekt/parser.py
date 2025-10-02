from .exceptions import *

template_start = "![["
template_end = "]]"

class StrPart:
    def __init__(self, start, end, template):
        self.start = start
        self.end = end
        self.template = template

def populate_str(var_dict, tofill):
        key = tofill.lower()
        if key not in var_dict:
            raise TemplateVariableNotFound(tofill)
        
        val = var_dict[key]
        if isinstance(val, str):
            return val
        elif callable(val):
            return val(var_dict)
        else:
            raise InvalidTemplateVariableType(tofill)

def parse(input, var_dict):
    str_parts = [
        StrPart(0, len(input), False)
    ]
    template_start = input.find(template_start)
    while template_start != -1:
        template_start += str_parts[-1].start
        str_parts[-1].end = template_start
        first_char_idx = template_start + len(template_start)
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
                break
        
        if last_char_idx == -1:
            raise PoorlyFormedTemplateVariable()
        
        template_start = input[str_parts[-1].start:].find(template_start)
    
    new_str = ""
    for p in str_parts:
        if p.template:
            new_str += populate_str(var_dict, input[p.start:p.end])
        else:
            new_str += input[p.start:p.end]

    return new_str