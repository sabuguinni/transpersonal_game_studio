#include "sacred_geometry.h"
#include <cmath>
#include <random>

namespace Architecture {

SacredGeometry::SacredGeometry() {
    initializePatterns();
}

void SacredGeometry::initializePatterns() {
    // Golden ratio and fibonacci sequences
    golden_ratio = (1.0f + sqrt(5.0f)) / 2.0f;
    
    // Sacred proportions for consciousness expansion
    sacred_proportions = {
        {1.0f, 1.0f},           // Unity
        {1.0f, golden_ratio},   // Golden rectangle
        {1.0f, sqrt(2.0f)},     // Sacred root rectangle
        {3.0f, 4.0f},           // Pythagorean
        {5.0f, 8.0f},           // Fibonacci
        {8.0f, 13.0f}           // Higher fibonacci
    };
    
    // Mandala patterns for floor designs
    mandala_patterns = {
        generateFlowerOfLife(),
        generateSriYantra(),
        generateMeditationCircle(),
        generateSacredSpiral()
    };
}

GeometryPattern SacredGeometry::generateFlowerOfLife() {
    GeometryPattern pattern;
    pattern.name = "Flower of Life";
    pattern.type = PatternType::MANDALA;
    pattern.consciousness_effect = 0.8f;
    
    // Generate overlapping circles in hexagonal pattern
    float radius = 1.0f;
    Vector3 center(0, 0, 0);
    
    // Central circle
    pattern.elements.push_back({center, radius, CircleType::SACRED});
    
    // Six surrounding circles
    for (int i = 0; i < 6; i++) {
        float angle = i * M_PI / 3.0f;
        Vector3 pos(cos(angle) * radius, 0, sin(angle) * radius);
        pattern.elements.push_back({pos, radius, CircleType::SACRED});
    }
    
    // Outer ring of 12 circles
    for (int i = 0; i < 12; i++) {
        float angle = i * M_PI / 6.0f;
        Vector3 pos(cos(angle) * radius * 2.0f, 0, sin(angle) * radius * 2.0f);
        pattern.elements.push_back({pos, radius, CircleType::SACRED});
    }
    
    return pattern;
}

GeometryPattern SacredGeometry::generateSriYantra() {
    GeometryPattern pattern;
    pattern.name = "Sri Yantra";
    pattern.type = PatternType::YANTRA;
    pattern.consciousness_effect = 0.9f;
    
    // Nine interlocking triangles
    float size = 1.0f;
    
    // Four upward triangles (Shiva)
    for (int i = 0; i < 4; i++) {
        Triangle tri;
        tri.type = TriangleType::UPWARD;
        tri.size = size * (1.0f - i * 0.2f);
        tri.rotation = i * 15.0f;
        pattern.triangles.push_back(tri);
    }
    
    // Five downward triangles (Shakti)
    for (int i = 0; i < 5; i++) {
        Triangle tri;
        tri.type = TriangleType::DOWNWARD;
        tri.size = size * (1.0f - i * 0.15f);
        tri.rotation = i * 12.0f;
        pattern.triangles.push_back(tri);
    }
    
    return pattern;
}

RoomLayout SacredGeometry::generateSacredRoom(RoomType type, float size) {
    RoomLayout layout;
    layout.type = type;
    layout.base_size = size;
    
    switch (type) {
        case RoomType::MEDITATION_CHAMBER:
            layout = generateMeditationChamber(size);
            break;
        case RoomType::CRYSTAL_SANCTUARY:
            layout = generateCrystalSanctuary(size);
            break;
        case RoomType::SOUND_TEMPLE:
            layout = generateSoundTemple(size);
            break;
        case RoomType::VISION_DOME:
            layout = generateVisionDome(size);
            break;
        case RoomType::HEALING_SPACE:
            layout = generateHealingSpace(size);
            break;
    }
    
    return layout;
}

RoomLayout SacredGeometry::generateMeditationChamber(float size) {
    RoomLayout layout;
    layout.type = RoomType::MEDITATION_CHAMBER;
    layout.shape = RoomShape::OCTAGONAL;
    layout.base_size = size;
    
    // Octagonal room for balanced energy flow
    layout.walls = generateOctagonalWalls(size);
    
    // Central mandala floor pattern
    layout.floor_pattern = generateFlowerOfLife();
    
    // Raised meditation platform in center
    Platform center_platform;
    center_platform.position = Vector3(0, 0.1f, 0);
    center_platform.radius = size * 0.3f;
    center_platform.height = 0.1f;
    center_platform.pattern = generateMeditationCircle();
    layout.platforms.push_back(center_platform);
    
    // Eight smaller platforms around perimeter
    for (int i = 0; i < 8; i++) {
        Platform platform;
        float angle = i * M_PI / 4.0f;
        platform.position = Vector3(
            cos(angle) * size * 0.7f,
            0.05f,
            sin(angle) * size * 0.7f
        );
        platform.radius = size * 0.15f;
        platform.height = 0.05f;
        layout.platforms.push_back(platform);
    }
    
    // Domed ceiling with sacred geometry
    layout.ceiling = generateSacredDome(size);
    
    // Soft ambient lighting
    layout.lighting = generateMeditationLighting();
    
    return layout;
}

RoomLayout SacredGeometry::generateCrystalSanctuary(float size) {
    RoomLayout layout;
    layout.type = RoomType::CRYSTAL_SANCTUARY;
    layout.shape = RoomShape::HEXAGONAL;
    layout.base_size = size;
    
    // Hexagonal room for crystal energy amplification
    layout.walls = generateHexagonalWalls(size);
    
    // Crystal formation patterns in floor
    layout.floor_pattern = generateCrystalPattern();
    
    // Central crystal cluster
    CrystalFormation center_crystal;
    center_crystal.position = Vector3(0, 0, 0);
    center_crystal.type = CrystalType::AMETHYST_CLUSTER;
    center_crystal.size = size * 0.4f;
    center_crystal.energy_level = 1.0f;
    layout.crystals.push_back(center_crystal);
    
    // Six perimeter crystals
    for (int i = 0; i < 6; i++) {
        CrystalFormation crystal;
        float angle = i * M_PI / 3.0f;
        crystal.position = Vector3(
            cos(angle) * size * 0.8f,
            0,
            sin(angle) * size * 0.8f
        );
        crystal.type = static_cast<CrystalType>(i % 6);
        crystal.size = size * 0.2f;
        crystal.energy_level = 0.7f;
        layout.crystals.push_back(crystal);
    }
    
    // Crystalline ceiling structure
    layout.ceiling = generateCrystalCeiling(size);
    
    // Color-changing crystal lighting
    layout.lighting = generateCrystalLighting();
    
    return layout;
}

std::vector<Wall> SacredGeometry::generateOctagonalWalls(float size) {
    std::vector<Wall> walls;
    
    for (int i = 0; i < 8; i++) {
        Wall wall;
        float angle1 = i * M_PI / 4.0f;
        float angle2 = (i + 1) * M_PI / 4.0f;
        
        wall.start = Vector3(cos(angle1) * size, 0, sin(angle1) * size);
        wall.end = Vector3(cos(angle2) * size, 0, sin(angle2) * size);
        wall.height = size * 0.8f;
        wall.material = WallMaterial::SACRED_STONE;
        wall.has_pattern = true;
        wall.pattern_type = PatternType::GEOMETRIC_RELIEF;
        
        walls.push_back(wall);
    }
    
    return walls;
}

LightingSystem SacredGeometry::generateMeditationLighting() {
    LightingSystem lighting;
    lighting.ambient_intensity = 0.3f;
    lighting.ambient_color = Color(0.9f, 0.8f, 0.7f); // Warm white
    
    // Central soft downlight
    Light center_light;
    center_light.position = Vector3(0, 3.0f, 0);
    center_light.type = LightType::SOFT_AREA;
    center_light.intensity = 0.5f;
    center_light.color = Color(1.0f, 0.9f, 0.8f);
    center_light.falloff = 2.0f;
    lighting.lights.push_back(center_light);
    
    // Perimeter accent lights
    for (int i = 0; i < 8; i++) {
        Light accent_light;
        float angle = i * M_PI / 4.0f;
        accent_light.position = Vector3(
            cos(angle) * 2.5f,
            1.5f,
            sin(angle) * 2.5f
        );
        accent_light.type = LightType::ACCENT;
        accent_light.intensity = 0.2f;
        accent_light.color = Color(0.8f, 0.9f, 1.0f); // Cool accent
        lighting.lights.push_back(accent_light);
    }
    
    return lighting;
}

float SacredGeometry::calculateConsciousnessResonance(const RoomLayout& room) {
    float resonance = 0.0f;
    
    // Base resonance from room shape
    switch (room.shape) {
        case RoomShape::CIRCULAR:
            resonance += 0.8f;
            break;
        case RoomShape::OCTAGONAL:
            resonance += 0.9f;
            break;
        case RoomShape::HEXAGONAL:
            resonance += 0.85f;
            break;
        default:
            resonance += 0.5f;
    }
    
    // Sacred proportion bonus
    float ratio = room.base_size / (room.base_size * 0.618f); // Golden ratio check
    if (abs(ratio - golden_ratio) < 0.1f) {
        resonance += 0.2f;
    }
    
    // Pattern complexity bonus
    resonance += room.floor_pattern.consciousness_effect * 0.3f;
    
    // Crystal energy amplification
    for (const auto& crystal : room.crystals) {
        resonance += crystal.energy_level * 0.1f;
    }
    
    // Lighting harmony
    resonance += calculateLightingHarmony(room.lighting) * 0.2f;
    
    return std::min(resonance, 1.0f);
}

} // namespace Architecture