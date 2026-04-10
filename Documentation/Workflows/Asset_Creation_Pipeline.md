# ASSET CREATION PIPELINE — TRANSPERSONAL GAME STUDIO

**Version:** 1.0  
**Last Updated:** March 2026  
**Owner:** Studio Director (#01)  

## OVERVIEW

This document defines the standardized asset creation pipeline for all agents in the Transpersonal Game Studio production chain. Every visual, audio, and technical asset must follow these procedures to ensure quality, consistency, and integration compatibility.

## PRODUCTION MANDATE COMPLIANCE

All asset creation must comply with **Production Mandate v3**:
- **Tool Budget:** Maximum 12 calls per session
- **Production Tools:** Minimum 4 calls required (heygen_create_video, generate_image, ue5_execute)
- **GitHub Integration:** Maximum 2 file writes for documentation
- **Language:** ALL game content in ENGLISH

## ASSET CATEGORIES

### 1. VISUAL ASSETS

#### Concept Art
- **Tool:** `generate_image`
- **Quality:** HD (high detail) for key assets, standard for iterations
- **Dimensions:** 
  - Character concepts: 1024x1792 (portrait)
  - Environment concepts: 1792x1024 (landscape)
  - UI mockups: 1024x1024 (square)
- **Storage:** Automatic to Supabase Storage
- **Naming:** Descriptive with agent prefix (e.g., "ENV_JurassicForest_Concept_v1")

#### In-Game Assets
- **Tool:** `ue5_execute` with asset import commands
- **Formats:** .fbx, .obj for 3D models; .png, .jpg for textures
- **Pipeline:** External creation → UE5 import → optimization → integration
- **LOD Requirements:** 3 levels minimum for performance

### 2. AUDIO ASSETS

#### Music & SFX
- **Tool:** `download_and_store` for external audio sources
- **Formats:** .wav (uncompressed), .ogg (compressed)
- **Quality:** 48kHz/24-bit for music, 44.1kHz/16-bit for SFX
- **Integration:** UE5 MetaSounds system

#### Voice & Dialogue
- **Tool:** `heygen_create_video` for character voice previews
- **Language:** ENGLISH ONLY for all game content
- **Format:** Extracted audio from video generation
- **Pipeline:** Script → HeyGen → Audio extraction → UE5 integration

### 3. VIDEO ASSETS

#### Dev Diaries & Marketing
- **Tool:** `heygen_create_video`
- **Specifications:**
  - Resolution: 1920x1080 (default)
  - Captions: Enabled for accessibility
  - Background: Branded colors (#1a2e1a for forest theme)
  - Duration: 2-5 minutes optimal

#### Cutscenes & Character Previews
- **Tool:** `heygen_create_video` for prototyping
- **Pipeline:** Script → Avatar generation → Review → Production
- **Integration:** Video files imported to UE5 Media Framework

### 4. TECHNICAL ASSETS

#### UE5 Scripts & Automation
- **Tool:** `ue5_execute` with Python scripts
- **Purpose:** Asset pipeline automation, system diagnostics, content generation
- **Standards:** Follow UE5 Python API best practices
- **Documentation:** All scripts must include inline comments

## QUALITY STANDARDS

### Visual Fidelity
- **Concept Art:** Photorealistic style for environmental pieces
- **Character Art:** Detailed textures with variation support
- **Environment Art:** Authentic Jurassic period accuracy
- **Lighting:** Warm, natural tones with dramatic contrast

### Technical Performance
- **Textures:** Power-of-2 dimensions, compressed formats
- **Models:** Optimized polygon count with LOD chains
- **Audio:** Compressed without quality loss
- **Video:** Efficient codecs for streaming

### Narrative Consistency
- **Visual Style:** Consistent with prehistoric survival theme
- **Character Design:** Realistic paleontologist aesthetic
- **Environment Design:** Scientifically accurate Jurassic flora/fauna
- **Audio Design:** Immersive natural soundscapes

## INTEGRATION WORKFLOW

### 1. Asset Creation
```
Agent Request → Tool Execution → Asset Generation → Quality Check
```

### 2. Storage & Organization
```
Asset Generation → Supabase Storage → URL Generation → Documentation
```

### 3. UE5 Integration
```
Asset URL → UE5 Import Command → Optimization → Testing → Deployment
```

### 4. Version Control
```
Asset Creation → GitHub Documentation → Version Tracking → Team Notification
```

## AGENT-SPECIFIC RESPONSIBILITIES

### Studio Director (#01)
- **Assets:** Concept art, dev diaries, system diagnostics
- **Tools:** All production tools for mandate compliance
- **Quality:** High-level vision consistency

### Engine Architect (#02)
- **Assets:** Technical architecture documents, system diagrams
- **Tools:** UE5 diagnostics, performance profiling
- **Quality:** Technical feasibility and optimization

### Environment Artist (#06)
- **Assets:** Terrain textures, vegetation models, atmospheric effects
- **Tools:** Image generation, UE5 asset import
- **Quality:** Photorealistic Jurassic environments

### Character Artist (#09)
- **Assets:** Character models, animations, variation systems
- **Tools:** MetaHuman integration, image generation
- **Quality:** Recognizable individual dinosaur variations

### Audio Agent (#16)
- **Assets:** Adaptive music, environmental SFX, voice acting
- **Tools:** HeyGen for voice, download for music libraries
- **Quality:** Immersive prehistoric soundscapes

## ERROR HANDLING

### Failed Asset Generation
1. **Retry:** Attempt generation with modified parameters
2. **Alternative:** Use different tool or approach
3. **Escalation:** Report to Studio Director for resolution
4. **Documentation:** Log all failures for pipeline improvement

### Integration Issues
1. **Validation:** Test all assets in UE5 before deployment
2. **Optimization:** Adjust parameters for performance requirements
3. **Compatibility:** Ensure cross-platform functionality
4. **Rollback:** Maintain previous versions for quick recovery

## PERFORMANCE TARGETS

### Asset Loading
- **Textures:** < 2MB per asset
- **Models:** < 50k triangles for hero assets
- **Audio:** < 10MB per music track
- **Video:** < 100MB per cutscene

### Runtime Performance
- **60fps:** PC target with high-quality assets
- **30fps:** Console target with optimized assets
- **Memory:** < 8GB total asset memory usage
- **Streaming:** Seamless LOD transitions

## APPROVAL PROCESS

### Automatic Approval
- Standard asset generation within quality parameters
- Technical assets for system diagnostics
- Iteration assets for development testing

### Manual Approval Required
- Hero assets for marketing and key gameplay
- Major technical architecture changes
- Assets requiring external resource purchases

---

**Document Owner:** Studio Director (#01)  
**Review Cycle:** Every 5 production cycles  
**Last Review:** PROD_CYCLE_003  
**Next Review:** PROD_CYCLE_008  

*Transpersonal Game Studio - Asset Creation Pipeline v1.0*