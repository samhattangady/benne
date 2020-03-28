#include "ui.h"

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

int get_op_id(df_operation_enum op) {
    switch (op) {
        case DISTANCE_FIELD_BLEND_ADD: return 0;
        case DISTANCE_FIELD_BLEND_SUBTRACT: return 1;
        case DISTANCE_FIELD_BLEND_UNION: return 2;
    }
    // TODO (23 Mar 2020 sam): What should this return?
    return -1;
}

void draw_node_editor(ui_state* state) {
    unsigned int index = state->active_index;
    ImGui::Begin("Node Editor");
    string name = string_from("Active Shape: ");
    append_sprintf(&name, " %i", index);
    ImGui::Button(name.text);
    df_node* node = &state->heap.nodes[index];
    df_operation* operation = &state->heap.operations[index];
    int op_id = get_op_id(operation->operation);
    ImGui::Combo("Op", &op_id, "Add\0Subtract\0Union\0");
    switch (op_id) {
        case 0: operation->operation = DISTANCE_FIELD_BLEND_ADD; break;
        case 1: operation->operation = DISTANCE_FIELD_BLEND_SUBTRACT; break;
        case 2: operation->operation = DISTANCE_FIELD_BLEND_UNION; break;
    }
    float* shape_data = state->heap.shapes[index].data;
    ImGui::SliderFloat("Ex", &(operation->extent), -0.0, 1.0);
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
    df_node* node = &state->heap.nodes[index];
    if (ImGui::Button("Set Active"))
        state->active_index = index;
    ImGui::SameLine();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(name.text)) {
        ImGui::SameLine();
        if (ImGui::Button("Add shape"))
            add_child(&state->heap, index);
        ImGui::SameLine();
        if (ImGui::Button("Delete shape"))
            dispose_node(&state->heap, index);
        for (int i=0; i<node->size; i++)
            draw_node(state, node->children[i]);
        ImGui::TreePop();
    }
    dispose_string(&name);
}

void draw_shape_selector(ui_state* state, unsigned int index) {
   ImGui::Begin("Edit shapes.");
   if (ImGui::Button("print shader")) {
       string shader = generate_frag_shader(&state->heap);
       printf("%s\n", shader.text);
       dispose_string(&shader);
   }
   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
   draw_node(state, index);
   ImGui::End();
}
