#include "consciousness_character_system.h"
#include "../core/consciousness_state.h"
#include "../lighting/consciousness_lighting.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

ConsciousnessCharacterSystem::ConsciousnessCharacterSystem() 
    : current_awareness_level(0.0f)
    , transformation_progress(0.0f)
    , emotional_resonance(0.5f) {
    
    // Initialize character states
    initializeCharacterStates();
    loadCharacterAssets();
}

void ConsciousnessCharacterSystem::initializeCharacterStates() {
    // Base human form - grounded, solid
    CharacterState base_state;
    base_state.opacity = 1.0f;
    base_state.ethereal_factor = 0.0f;
    base_state.energy_glow = 0.1f;
    base_state.particle_density = 0.0f;
    base_state.form_stability = 1.0f;
    character_states["base"] = base_state;
    
    // Awakening state - beginning to transcend
    CharacterState awakening_state;
    awakening_state.opacity = 0.9f;
    awakening_state.ethereal_factor = 0.2f;
    awakening_state.energy_glow = 0.3f;
    awakening_state.particle_density = 0.1f;
    awakening_state.form_stability = 0.9f;
    character_states["awakening"] = awakening_state;
    
    // Expanded state - higher consciousness
    CharacterState expanded_state;
    expanded_state.opacity = 0.7f;
    expanded_state.ethereal_factor = 0.5f;
    expanded_state.energy_glow = 0.6f;
    expanded_state.particle_density = 0.3f;
    expanded_state.form_stability = 0.7f;
    character_states["expanded"] = expanded_state;
    
    // Transcendent state - pure consciousness
    CharacterState transcendent_state;
    transcendent_state.opacity = 0.4f;
    transcendent_state.ethereal_factor = 0.8f;
    transcendent_state.energy_glow = 0.9f;
    transcendent_state.particle_density = 0.6f;
    transcendent_state.form_stability = 0.4f;
    character_states["transcendent"] = transcendent_state;
    
    // Unity state - merged with universal consciousness
    CharacterState unity_state;
    unity_state.opacity = 0.2f;
    unity_state.ethereal_factor = 1.0f;
    unity_state.energy_glow = 1.0f;
    unity_state.particle_density = 0.8f;
    unity_state.form_stability = 0.2f;
    character_states["unity"] = unity_state;
}

void ConsciousnessCharacterSystem::updateCharacterVisualization(float delta_time, const ConsciousnessState& consciousness) {
    // Update awareness level based on consciousness state
    float target_awareness = consciousness.getAwarenessLevel();
    current_awareness_level = lerp(current_awareness_level, target_awareness, delta_time * 2.0f);
    
    // Update emotional resonance
    emotional_resonance = consciousness.getEmotionalResonance();
    
    // Determine current character state based on awareness
    std::string target_state = determineCharacterState(current_awareness_level);
    
    // Smooth transition between states
    updateStateTransition(target_state, delta_time);
    
    // Update visual effects
    updateEnergyField(delta_time);
    updateParticleEffects(delta_time);
    updateFormMorphing(delta_time);
    updateAuraVisualization(delta_time);
}

std::string ConsciousnessCharacterSystem::determineCharacterState(float awareness_level) {
    if (awareness_level < 0.2f) return "base";
    else if (awareness_level < 0.4f) return "awakening";
    else if (awareness_level < 0.6f) return "expanded";
    else if (awareness_level < 0.8f) return "transcendent";
    else return "unity";
}

void ConsciousnessCharacterSystem::updateStateTransition(const std::string& target_state, float delta_time) {
    if (target_state != current_state) {
        transformation_progress += delta_time * 1.5f; // Transition speed
        
        if (transformation_progress >= 1.0f) {
            current_state = target_state;
            transformation_progress = 0.0f;
        }
        
        // Blend between current and target states
        blendCharacterStates(current_state, target_state, transformation_progress);
    }
}

void ConsciousnessCharacterSystem::blendCharacterStates(const std::string& from_state, 
                                                       const std::string& to_state, 
                                                       float blend_factor) {
    auto from_it = character_states.find(from_state);
    auto to_it = character_states.find(to_state);
    
    if (from_it != character_states.end() && to_it != character_states.end()) {
        const CharacterState& from = from_it->second;
        const CharacterState& to = to_it->second;
        
        // Interpolate all visual properties
        current_visual_state.opacity = lerp(from.opacity, to.opacity, blend_factor);
        current_visual_state.ethereal_factor = lerp(from.ethereal_factor, to.ethereal_factor, blend_factor);
        current_visual_state.energy_glow = lerp(from.energy_glow, to.energy_glow, blend_factor);
        current_visual_state.particle_density = lerp(from.particle_density, to.particle_density, blend_factor);
        current_visual_state.form_stability = lerp(from.form_stability, to.form_stability, blend_factor);
    }
}

void ConsciousnessCharacterSystem::updateEnergyField(float delta_time) {
    // Pulsing energy field that responds to consciousness level
    float pulse_frequency = 1.0f + current_awareness_level * 3.0f;
    float pulse_intensity = sin(GetTime() * pulse_frequency) * 0.5f + 0.5f;
    
    energy_field_intensity = current_visual_state.energy_glow * pulse_intensity;
    
    // Color shifts based on emotional state
    glm::vec3 base_color = glm::vec3(0.3f, 0.6f, 1.0f); // Calm blue
    glm::vec3 expanded_color = glm::vec3(1.0f, 0.8f, 0.3f); // Golden
    glm::vec3 transcendent_color = glm::vec3(0.9f, 0.3f, 1.0f); // Violet
    
    if (current_awareness_level < 0.5f) {
        energy_field_color = lerp(base_color, expanded_color, current_awareness_level * 2.0f);
    } else {
        energy_field_color = lerp(expanded_color, transcendent_color, (current_awareness_level - 0.5f) * 2.0f);
    }
}

void ConsciousnessCharacterSystem::updateParticleEffects(float delta_time) {
    // Consciousness particles that emanate from the character
    particle_system.density = current_visual_state.particle_density;
    particle_system.emission_rate = current_awareness_level * 100.0f;
    
    // Particle behavior changes with consciousness level
    if (current_awareness_level < 0.3f) {
        // Grounded particles - slow, downward
        particle_system.velocity = glm::vec3(0.0f, -0.5f, 0.0f);
        particle_system.spread = 0.5f;
    } else if (current_awareness_level < 0.7f) {
        // Rising particles - upward movement
        particle_system.velocity = glm::vec3(0.0f, 1.0f, 0.0f);
        particle_system.spread = 1.0f;
    } else {
        // Transcendent particles - omnidirectional, flowing
        particle_system.velocity = glm::vec3(0.0f, 2.0f, 0.0f);
        particle_system.spread = 2.0f;
        particle_system.flow_pattern = "spiral";
    }
    
    particle_system.update(delta_time);
}

void ConsciousnessCharacterSystem::updateFormMorphing(float delta_time) {
    // Character form becomes more ethereal with higher consciousness
    float morph_intensity = 1.0f - current_visual_state.form_stability;
    
    // Vertex displacement for ethereal effect
    for (auto& vertex : character_mesh.vertices) {
        float noise_factor = perlinNoise(vertex.position + glm::vec3(GetTime() * 0.5f));
        vertex.position += vertex.normal * noise_factor * morph_intensity * 0.1f;
    }
    
    // Edge softening for transparency effects
    character_mesh.edge_softness = current_visual_state.ethereal_factor;
    
    // Update mesh buffers
    character_mesh.updateBuffers();
}

void ConsciousnessCharacterSystem::updateAuraVisualization(float delta_time) {
    // Aura size and intensity based on consciousness expansion
    aura_radius = 1.0f + current_awareness_level * 3.0f;
    aura_intensity = current_visual_state.energy_glow;
    
    // Aura layers for depth
    aura_layers.clear();
    int layer_count = static_cast<int>(current_awareness_level * 5.0f) + 1;
    
    for (int i = 0; i < layer_count; ++i) {
        AuraLayer layer;
        layer.radius = aura_radius * (1.0f + i * 0.3f);
        layer.opacity = aura_intensity * (1.0f - i * 0.2f);
        layer.color = energy_field_color;
        layer.rotation_speed = 0.5f + i * 0.2f;
        aura_layers.push_back(layer);
    }
}

void ConsciousnessCharacterSystem::renderCharacter() {
    // Set character opacity and blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render base character mesh
    character_shader.use();
    character_shader.setFloat("opacity", current_visual_state.opacity);
    character_shader.setFloat("ethereal_factor", current_visual_state.ethereal_factor);
    character_shader.setVec3("energy_color", energy_field_color);
    character_shader.setFloat("energy_intensity", energy_field_intensity);
    
    character_mesh.render();
    
    // Render aura layers
    renderAura();
    
    // Render particle effects
    particle_system.render();
    
    // Render energy field
    renderEnergyField();
}

void ConsciousnessCharacterSystem::renderAura() {
    aura_shader.use();
    
    for (const auto& layer : aura_layers) {
        aura_shader.setFloat("radius", layer.radius);
        aura_shader.setFloat("opacity", layer.opacity);
        aura_shader.setVec3("color", layer.color);
        aura_shader.setFloat("time", GetTime() * layer.rotation_speed);
        
        // Render aura sphere
        aura_mesh.render();
    }
}

void ConsciousnessCharacterSystem::renderEnergyField() {
    // Volumetric energy field rendering
    energy_field_shader.use();
    energy_field_shader.setFloat("intensity", energy_field_intensity);
    energy_field_shader.setVec3("color", energy_field_color);
    energy_field_shader.setFloat("awareness_level", current_awareness_level);
    energy_field_shader.setFloat("time", GetTime());
    
    energy_field_mesh.render();
}

float ConsciousnessCharacterSystem::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

glm::vec3 ConsciousnessCharacterSystem::lerp(const glm::vec3& a, const glm::vec3& b, float t) {
    return a + t * (b - a);
}

float ConsciousnessCharacterSystem::perlinNoise(const glm::vec3& position) {
    // Simple Perlin noise implementation for morphing effects
    // This would typically use a proper noise library
    return sin(position.x * 0.1f) * cos(position.y * 0.1f) * sin(position.z * 0.1f);
}

void ConsciousnessCharacterSystem::loadCharacterAssets() {
    // Load character mesh and textures
    character_mesh.loadFromFile("assets/characters/base_character.obj");
    
    // Load shaders
    character_shader.loadFromFiles("shaders/character_vertex.glsl", "shaders/character_fragment.glsl");
    aura_shader.loadFromFiles("shaders/aura_vertex.glsl", "shaders/aura_fragment.glsl");
    energy_field_shader.loadFromFiles("shaders/energy_field_vertex.glsl", "shaders/energy_field_fragment.glsl");
    
    // Initialize aura and energy field meshes
    aura_mesh.createSphere(1.0f, 32, 16);
    energy_field_mesh.createSphere(1.5f, 16, 8);
}