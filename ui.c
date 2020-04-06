#include "ui.h"

/*
void add_child(df_heap* heap, unsigned int parent_index) {
    attach_node(heap,
        generate_sphere(
            heap,
            0.0, 0.0, 0.0,
            0.1, 1.0
        ),
        { DISTANCE_FIELD_BLEND_ADD, 0.2 },
        parent_index
    );
}

void float_input(float** shape_data_ptr, unsigned int index, char* label, float min, float max) {
    float* shape_data = *shape_data_ptr;
    static char buffer[64] = "";
    sprintf(buffer, "%f", shape_data[index]);
    ImGui::SetNextItemWidth(200);
    ImGui::InputText(label, buffer, 64, ImGuiInputTextFlags_CharsDecimal);
    sscanf(buffer, "%f", &shape_data[index]);
}

int get_op_id(df_operation_enum op) {
    switch (op) {
        case DISTANCE_FIELD_BLEND_ADD: return 0;
        case DISTANCE_FIELD_BLEND_SUBTRACT: return 1;
        case DISTANCE_FIELD_BLEND_UNION: return 2;
    }
    // TODO (23 Mar 2020 sam): What should this return?
    return -1;
}

int get_shape_type(df_shape_enum shape) {
    switch (shape) {
        case EMPTY: return 0;
        case SPHERE: return 1;
        case ROUNDED_RECTANGLE: return 2;
    }
    // TODO (29 Mar 2020 sam): What should this return?
    return -1;
}

void draw_node_editor(ui_state* state) {
    unsigned int index = state->active_index;
    ImGui::Begin("Node Editor");
    string name = string_from("Active Shape: ");
    append_sprintf(&name, " %i", index);
    ImGui::Text(name.text);
    ImGui::SameLine();
    df_node* node = &state->heap->nodes[index];
    df_shape* shape = &state->heap->shapes[index];
    df_operation* operation = &state->heap->operations[index];
    float* shape_data = state->heap->shapes[index].data;

    int shape_type = get_shape_type(shape->type);
    ImGui::SetNextItemWidth(200);
    ImGui::Combo("Shape", &shape_type, "Empty\0Sphere\0Box\0");
    ImGui::SameLine();
    switch (shape_type) {
        case 0: shape->type = EMPTY; break;
        case 1: shape->type = SPHERE; break;
        case 2: shape->type = ROUNDED_RECTANGLE; break;
    }
    int op_id = get_op_id(operation->operation);
    ImGui::SetNextItemWidth(200);
    ImGui::Combo("Op", &op_id, "Add\0Subtract\0Union\0");
    ImGui::SameLine();
    switch (op_id) {
        case 0: operation->operation = DISTANCE_FIELD_BLEND_ADD; break;
        case 1: operation->operation = DISTANCE_FIELD_BLEND_SUBTRACT; break;
        case 2: operation->operation = DISTANCE_FIELD_BLEND_UNION; break;
    }
    ImGui::SetNextItemWidth(200);
    ImGui::SliderFloat("Ex", &(operation->extent), -0.0, 1.0);

    if (ImGui::TreeNode("Text Inputs")) {
        float_input(&shape_data, 0, "x_pos", -1.0, 1.0);
        ImGui::SameLine();
        float_input(&shape_data, 1, "y_pos", -1.0, 1.0);
        ImGui::SameLine();
        float_input(&shape_data, 2, "z_pos", -1.0, 1.0);
        ImGui::Separator();
        float_input(&shape_data, 3, "x_rad", -1.0, 1.0);
        ImGui::SameLine();
        float_input(&shape_data, 4, "y_rad", -1.0, 1.0);
        ImGui::SameLine();
        float_input(&shape_data, 5, "z_rad", -1.0, 1.0);
        ImGui::Separator();
        float_input(&shape_data, 6, "x_siz", -1.0, 1.0);
        ImGui::SameLine();
        float_input(&shape_data, 7, "y_siz", -1.0, 1.0);
        ImGui::SameLine();
        float_input(&shape_data, 8, "z_siz", -1.0, 1.0);
        ImGui::Separator();
        float_input(&shape_data, 9, "radiu", -1.0, 1.0);
        ImGui::Separator();
        ImGui::TreePop();
    }
    ImGui::SliderFloat3("Position", &shape_data[0], -1.0, 1.0);
    ImGui::SliderFloat3("Angle", &shape_data[3], -4.0, 4.0);
    ImGui::SliderFloat3("Size",  &shape_data[6],  0.0, 1.0);
    ImGui::SliderFloat("Radius", &shape_data[9], -0.0, 2.0);
    dispose_string(&name);
    ImGui::End();
}

void draw_node(ui_state* state, unsigned int index) {
    string name = string_from("Shape");
    append_sprintf(&name, " %i", index);
    df_node* node = &state->heap->nodes[index];
    if (ImGui::Button(name.text))
        state->active_index = index;
    ImGui::SameLine();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(name.text)) {
        ImGui::SameLine();
        if (ImGui::Button("Add shape")) add_child(state->heap, index);
        ImGui::SameLine();
        // TODO (29 Mar 2020 sam): Create confirmation dialog
        if (ImGui::Button("Delete shape")) dispose_node(state->heap, index);
        for (int i=0; i<node->size; i++) draw_node(state, node->children[i]);
        ImGui::TreePop();
    }
    dispose_string(&name);
}

void draw_shape_selector(ui_state* state, unsigned int index) {
   ImGui::Begin("Edit shapes.");
   if (ImGui::Button("print shader")) {
       string shader = generate_frag_shader(state->heap);
       printf("%s\n", shader.text);
       dispose_string(&shader);
   }
   ImGui::SameLine();
   if (ImGui::Button("print tree")) {
       print_node(state->heap, 0);
       printf("-----\n");
   }
   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
   draw_node(state, index);
   ImGui::End();
}
*/
