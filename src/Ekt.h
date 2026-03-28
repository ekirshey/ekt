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
    bool resolve_template(const std::string& template_name);
    bool template_exists(const std::string& template_name);
    std::vector<std::string> available_templates();
private:
    void get_user_input_variables(Context& context, const Template& selected_template);
    void get_missing_variables(Context& context, const Template& selected_template, const ParsedTemplateString& parsed_template);
    bool resolve_functions(Context& context, const Template& selected_template);
    bool run_post_commands(const Template& selected_template);

    std::unordered_map<std::string, Template> m_templates;
    std::unordered_map<std::string, std::string> m_global_context;
};
