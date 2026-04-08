// Copyright Transpersonal Game Studio. All Rights Reserved.
// Architectural Storytelling System - Every structure tells a story
// Agent #07 - Architecture & Interior Agent
// CYCLE_ID: REQ-20260408-MKT-004

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/DecalActor.h"
#include "ArchitectureTypes.h"
#include "JurassicArchitectureCore.h"
#include "../WorldGeneration/JurassicBiomeManager.h"
#include "../EnvironmentArt/EnvironmentArtManager.h"
#include "ArchitecturalStorytellingSystem.generated.h"

/**
 * @brief Architectural Storytelling System - Transpersonal Game Studio
 * 
 * "Um interior vazio é uma mentira. Nenhum ser humano vive ou trabalha num espaço 
 * sem deixar marcas do que é, do que precisa, do que teme." - Gaston Bachelard
 * 
 * Core Philosophy (Stewart Brand + Gaston Bachelard):
 * - Buildings show time in layers (construction, habitation, abandonment)
 * - Space has memory - every room tells who lived there and what happened
 * - Architecture is archaeology - we create artifacts for future discovery
 * - No empty interiors - every space must answer: who lived here, what happened?
 * 
 * System Responsibilities:
 * - Generate prehistoric human settlements with believable stories
 * - Create layered interiors that show evidence of human habitation
 * - Place storytelling props that reveal inhabitant personalities and fates
 * - Manage architectural decay and nature reclamation over time
 * - Integrate seamlessly with environment art and lighting systems
 * 
 * Technical Integration:
 * - Uses PCG Framework for structure placement and interior generation
 * - Integrates with Biome Manager for location-appropriate architecture
 * - Works with Environment Art Manager for material and prop consistency
 * - Prepares lighting anchor points for Lighting Agent (#8)
 * - Provides quest hooks and narrative elements for Quest Designer (#14)
 * 
 * @author Architecture & Interior Agent — Agent #7
 * @version 1.0 — March 2026
 */

class UPCGGraph;
class UPCGComponent;
class AJurassicBiomeManager;
class AEnvironmentArtManager;
class UStaticMesh;
class UMaterialInterface;
class USoundCue;
class UParticleSystem;

/** Human habitation evidence types */
UENUM(BlueprintType)
enum class EHabitationEvidence : uint8
{
    // Personal belongings
    SleepingArea        UMETA(DisplayName = "Sleeping Area - Bedding and Personal Items"),
    CookingHearth       UMETA(DisplayName = "Cooking Hearth - Fire pit and Food Remains"),
    ToolWorkstation     UMETA(DisplayName = "Tool Workstation - Crafting Area"),
    WeaponRack          UMETA(DisplayName = "Weapon Rack - Spears and Clubs"),
    StorageCache        UMETA(DisplayName = "Storage Cache - Food and Material Storage"),
    
    // Daily life traces
    WornFloorPath       UMETA(DisplayName = "Worn Floor Path - Foot Traffic Patterns"),
    HandPrints          UMETA(DisplayName = "Hand Prints - Touch Marks on Walls"),
    SootMarks           UMETA(DisplayName = "Soot Marks - Fire and Smoke Stains"),
    WearPatterns        UMETA(DisplayName = "Wear Patterns - Usage Evidence"),
    RepairMarks         UMETA(DisplayName = "Repair Marks - Maintenance Evidence"),
    
    // Emotional traces
    ChildrenToys        UMETA(DisplayName = "Children Toys - Family Evidence"),
    ArtisticMarks       UMETA(DisplayName = "Artistic Marks - Cave Paintings"),
    RitualSpace         UMETA(DisplayName = "Ritual Space - Spiritual Activity"),
    MemorialCorner      UMETA(DisplayName = "Memorial Corner - Remembrance Area"),
    ComfortItems        UMETA(DisplayName = "Comfort Items - Emotional Anchors"),
    
    // Departure evidence
    PackedBelongings    UMETA(DisplayName = "Packed Belongings - Organized Departure"),
    AbandonedItems      UMETA(DisplayName = "Abandoned Items - Hasty Departure"),
    BarricadeMarks      UMETA(DisplayName = "Barricade Marks - Defensive Departure"),
    HiddenStash         UMETA(DisplayName = "Hidden Stash - Planned Return"),
    DestructionMarks    UMETA(DisplayName = "Destruction Marks - Violent Departure")
};

/** Architectural decay stages */
UENUM(BlueprintType)
enum class EDecayStage : uint8
{
    Fresh               UMETA(DisplayName = "Fresh - Recently Abandoned (Days)"),
    Early               UMETA(DisplayName = "Early Decay - Weeks to Months"),
    Moderate            UMETA(DisplayName = "Moderate Decay - Months to Years"),
    Advanced            UMETA(DisplayName = "Advanced Decay - Years to Decades"),
    Ruins               UMETA(DisplayName = "Ruins - Decades to Centuries"),
    Archaeological      UMETA(DisplayName = "Archaeological - Centuries Plus")
};

/** Interior layout functional zones */
UENUM(BlueprintType)
enum class EInteriorZone : uint8
{
    Entrance            UMETA(DisplayName = "Entrance - Transition and Defense"),
    Living              UMETA(DisplayName = "Living - Daily Activity Center"),
    Sleeping            UMETA(DisplayName = "Sleeping - Rest and Privacy"),
    Cooking             UMETA(DisplayName = "Cooking - Food Preparation"),
    Storage             UMETA(DisplayName = "Storage - Resource Management"),
    Work                UMETA(DisplayName = "Work - Tool and Weapon Crafting"),
    Ritual              UMETA(DisplayName = "Ritual - Spiritual and Social"),
    Emergency           UMETA(DisplayName = "Emergency - Escape and Hiding")
};

/** Storytelling prop configuration */
USTRUCT(BlueprintType)
struct FStorytellingProp
{
    GENERATED_BODY()

    /** Prop identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    EHabitationEvidence EvidenceType = EHabitationEvidence::SleepingArea;

    /** Visual representation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UMaterialInterface> PropMaterial;

    /** Placement rules */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    EInteriorZone PreferredZone = EInteriorZone::Living;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector RelativePosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    /** Storytelling data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 StoryImportance = 1; // 1-10 scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsQuestRelated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsInteractable = false;

    /** Decay behavior */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
    EDecayStage MinDecayStage = EDecayStage::Fresh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
    EDecayStage MaxDecayStage = EDecayStage::Archaeological;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
    bool bSurvivesDecay = false;

    /** Atmospheric effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TSoftObjectPtr<UParticleSystem> AmbientParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor AmbientLightTint = FLinearColor::White;

    /** Spawn probability based on inhabitant story */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TMap<EInhabitationStory, float> SpawnProbabilityByStory;
};

/** Interior zone configuration */
USTRUCT(BlueprintType)
struct FInteriorZoneConfig
{
    GENERATED_BODY()

    /** Zone identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EInteriorZone ZoneType = EInteriorZone::Living;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FString ZoneName;

    /** Spatial definition */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    FVector ZoneExtents = FVector(200.0f, 200.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    float ZoneRadius = 200.0f;

    /** Functional requirements */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
    TArray<EHabitationEvidence> RequiredEvidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
    TArray<EHabitationEvidence> OptionalEvidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
    int32 MinProps = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
    int32 MaxProps = 5;

    /** Environmental factors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bRequiresNaturalLight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bRequiresVentilation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bRequiresWaterAccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bRequiresFireAccess = false;

    /** Traffic patterns */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    TArray<EInteriorZone> ConnectedZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
    float TrafficIntensity = 1.0f; // Affects wear patterns

    /** Privacy and security */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
    bool bIsPrivateSpace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
    bool bHasSecretAccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
    bool bIsDefensivePosition = false;
};

/** Complete interior story configuration */
USTRUCT(BlueprintType)
struct FInteriorStoryConfig
{
    GENERATED_BODY()

    /** Story metadata */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    EInhabitationStory InhabitationType = EInhabitationStory::ActiveFamily;

    /** Inhabitants */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitants")
    int32 AdultCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitants")
    int32 ChildCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitants")
    int32 ElderCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitants")
    TArray<FString> InhabitantPersonalities;

    /** Timeline */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 InhabitationDurationDays = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 DaysAbandonedMin = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    int32 DaysAbandonedMax = 365;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
    EDecayStage CurrentDecayStage = EDecayStage::Early;

    /** Departure circumstances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Departure")
    FString DepartureReason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Departure")
    bool bPlannedDeparture = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Departure")
    bool bViolentDeparture = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Departure")
    bool bIntentionToReturn = false;

    /** Zone configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TArray<FInteriorZoneConfig> InteriorZones;

    /** Props to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FStorytellingProp> StoryProps;

    /** Environmental storytelling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FString> VisualClues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FString> AudioClues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FString OverallMood;

    /** Quest integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasQuestItems = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> QuestClues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsKeyLocation = false;
};

/**
 * @brief Architectural Storytelling System Manager
 * 
 * Manages the generation and maintenance of story-driven architectural interiors.
 * Every structure tells a complete story through environmental storytelling.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStorytellingSystem : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStorytellingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Core components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* PCGComponent;

    /** Integration with other systems */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TSoftObjectPtr<AJurassicBiomeManager> BiomeManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TSoftObjectPtr<AEnvironmentArtManager> EnvironmentArtManager;

    /** Story configuration library */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Configuration")
    TArray<FInteriorStoryConfig> StoryTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Configuration")
    TArray<FStorytellingProp> PropLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Configuration")
    TMap<EStructureType, TArray<FInteriorStoryConfig>> StoriesByStructureType;

    /** Decay and weathering settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay System")
    float DecayProgressionRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay System")
    bool bEnableRealTimeDecay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay System")
    float NatureReclamationRate = 0.1f;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float InteriorDetailCullDistance = 5000.0f; // 50m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveInteriors = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODForProps = true;

    /** Debug and visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowStoryDebugInfo = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowZoneBounds = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowTrafficPatterns = false;

public:
    /** Core storytelling functions */
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FInteriorStoryConfig GenerateStoryForStructure(EStructureType StructureType, EJurassicBiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PopulateInteriorWithStory(AActor* StructureActor, const FInteriorStoryConfig& StoryConfig);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FStorytellingProp> SelectPropsForStory(const FInteriorStoryConfig& StoryConfig);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void ApplyDecayToInterior(AActor* StructureActor, EDecayStage DecayStage);

    /** Zone management */
    UFUNCTION(BlueprintCallable, Category = "Interior Layout")
    TArray<FInteriorZoneConfig> GenerateZoneLayout(EStructureType StructureType, int32 InhabitantCount);

    UFUNCTION(BlueprintCallable, Category = "Interior Layout")
    void PlacePropsInZone(const FInteriorZoneConfig& ZoneConfig, const TArray<FStorytellingProp>& Props, AActor* StructureActor);

    UFUNCTION(BlueprintCallable, Category = "Interior Layout")
    FVector FindOptimalPropPlacement(const FStorytellingProp& Prop, const FInteriorZoneConfig& Zone, AActor* StructureActor);

    /** Environmental storytelling */
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void AddWearPatterns(AActor* StructureActor, const FInteriorStoryConfig& StoryConfig);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void AddSootAndStains(AActor* StructureActor, const TArray<FInteriorZoneConfig>& Zones);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void AddPersonalTouches(AActor* StructureActor, const FInteriorStoryConfig& StoryConfig);

    /** Decay simulation */
    UFUNCTION(BlueprintCallable, Category = "Decay Simulation")
    void ProgressDecay(AActor* StructureActor, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Decay Simulation")
    void AddNatureReclamation(AActor* StructureActor, EDecayStage DecayStage);

    UFUNCTION(BlueprintCallable, Category = "Decay Simulation")
    void UpdateMaterialsForDecay(UStaticMeshComponent* MeshComponent, EDecayStage DecayStage);

    /** Integration functions */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterWithBiomeManager();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SyncWithEnvironmentArt();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FVector> GetLightingAnchorPoints(AActor* StructureActor);

    /** Debug and analysis */
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void AnalyzeStoryCoherence();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidateInteriorLayouts();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawStoryDebugInfo(AActor* StructureActor);

private:
    /** Internal story generation */
    FInteriorStoryConfig CreateRandomStory(EStructureType StructureType, EJurassicBiomeType BiomeType);
    TArray<FString> GenerateInhabitantPersonalities(int32 Count);
    FString GenerateDepartureReason(bool bPlanned, bool bViolent);
    
    /** Internal prop management */
    void SpawnPropActor(const FStorytellingProp& Prop, const FVector& Location, const FRotator& Rotation, AActor* ParentActor);
    void ApplyPropDecay(AActor* PropActor, EDecayStage DecayStage);
    
    /** Internal zone management */
    FInteriorZoneConfig CreateZoneConfig(EInteriorZone ZoneType, const FVector& Center, float Radius);
    bool ValidateZonePlacement(const FInteriorZoneConfig& Zone, AActor* StructureActor);
    
    /** Internal decay management */
    void UpdateDecayMaterials(UStaticMeshComponent* Component, float DecayProgress);
    void SpawnDecayParticles(const FVector& Location, EDecayStage DecayStage);
    
    /** Performance optimization */
    void OptimizeInteriorLOD(AActor* StructureActor, float DistanceToPlayer);
    void CullDistantInteriors();
    
    /** Story coherence validation */
    bool ValidateStoryLogic(const FInteriorStoryConfig& StoryConfig);
    float CalculateStoryCoherenceScore(const FInteriorStoryConfig& StoryConfig);
};