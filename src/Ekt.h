#pragma once

#include <string>
#include <unordered_map>

#include "Context.h"

#include "ParsedTemplateString.h"
#include "Template.h"


class Ekt
{
public:
    void add_template(const std::string& name, const Template& ekt_template);
    void add_global_var(const std::string& key, const std::string& value);
    bool resolve_template(const std::string& template_name);
    bool template_exists(const std::string& template_name);
    std::vector<std::string> available_templates();
private:
    void get_user_input_variables(Context& context, const Template& selected_template);
    void get_missing_variables(Context& context, const Template& selected_template, const ParsedTemplateString& parsed_template);
    bool resolve_functions(Context& context, const Template& selected_template);

    bool run_post_commands(const Context& context, const std::vector<ParsedTemplateString>& parsed_commands);

    std::unordered_map<std::string, Template> m_templates;
    Context m_global_context;
};
