#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Landscape.h"
#include "JurassicVegetationSystem.h"
#include "JurassicMaterialSystem.h"
#include "JurassicEnvironmentArtist.generated.h"

UENUM(BlueprintType)
enum class EEnvironmentDetailLevel : uint8
{
    Hero        UMETA(DisplayName = "Hero Detail - Player Focus Areas"),
    High        UMETA(DisplayName = "High Detail - Near Player"),
    Medium      UMETA(DisplayName = "Medium Detail - Mid Distance"),
    Low         UMETA(DisplayName = "Low Detail - Far Distance"),
    Background  UMETA(DisplayName = "Background - Skybox Detail")
};

UENUM(BlueprintType)
enum class EStorytellingLayer : uint8
{
    Ancient         UMETA(DisplayName = "Ancient - Geological Features"),
    Prehistoric     UMETA(DisplayName = "Prehistoric - Dinosaur Activity"),
    Recent          UMETA(DisplayName = "Recent - Fresh Events"),
    PlayerImpact    UMETA(DisplayName = "Player Impact - Dynamic Changes"),
    Mystery         UMETA(DisplayName = "Mystery - Gema Clues")
};

USTRUCT(BlueprintType)
struct FEnvironmentProp
{
    GENERATED_BODY()

    // Static mesh for this prop
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UStaticMesh* PropMesh;

    // Material overrides for this prop
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UMaterialInterface*> MaterialOverrides;

    // Scale variation range
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    // Rotation randomization
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomRotation = true;

    // Spawn probability (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float SpawnProbability = 1.0f;

    // Clustering settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bClusterSpawning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ClusterSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClusterRadius = 200.0f;

    // Environmental requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSlope = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredDistanceToWater = 0.0f; // Negative = near water, positive = away from water

    // Storytelling context
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStorytellingLayer StoryLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StoryContext;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentDetailLevel DetailLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CullDistance = 5000.0f;

    FEnvironmentProp()
    {
        PropMesh = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FRockFormation
{
    GENERATED_BODY()

    // Base rock meshes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UStaticMesh*> RockMeshes;

    // Formation pattern
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinRocks = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxRocks = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FormationRadius = 500.0f;

    // Size variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ScaleRange = FVector2D(0.5f, 2.0f);

    // Material variations for weathering
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UMaterialInterface*> WeatheringMaterials;

    // Moss/vegetation on rocks
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowVegetationGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UStaticMesh*> RockVegetation;
};

USTRUCT(BlueprintType)
struct FNarrativeEnvironmentCluster
{
    GENERATED_BODY()

    // Cluster identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClusterName;

    // Story this cluster tells
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NarrativeDescription;

    // Props that make up this story
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnvironmentProp> StoryProps;

    // Decals for this story
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UMaterialInterface*> StoryDecals;

    // Audio cues for this story
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundCue* StoryAmbientSound;

    // Particle effects for this story
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UParticleSystem*> StoryParticles;

    // Size of the narrative area
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClusterRadius = 1000.0f;

    // Intensity of the storytelling (affects prop density)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.1f, ClampMax = 2.0f))
    float StoryIntensity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UJurassicEnvironmentProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    // Biome this profile is for
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EJurassicBiomeType TargetBiome;

    // Rock formations for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRockFormation> RockFormations;

    // Environmental props (fallen logs, bones, crystals, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnvironmentProp> EnvironmentProps;

    // Ground scatter (small rocks, sticks, leaves)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnvironmentProp> GroundScatter;

    // Water features (streams, pools, waterfalls)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnvironmentProp> WaterFeatures;

    // Narrative clusters for environmental storytelling
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarrativeEnvironmentCluster> NarrativeClusters;

    // Atmospheric particles for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UParticleSystem*> AtmosphericParticles;

    // Ambient lighting adjustments
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientLightTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientLightIntensity = 1.0f;

    // Fog settings for atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UJurassicEnvironmentArtist : public UActorComponent
{
    GENERATED_BODY()

public:
    UJurassicEnvironmentArtist();

protected:
    virtual void BeginPlay() override;

public:
    // Main environment population function
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateEnvironment(FVector Center, float Radius, EJurassicBiomeType BiomeType, EEnvironmentDetailLevel DetailLevel);

    // Specialized functions for different environment elements
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlaceRockFormations(FVector Center, float Radius, EJurassicBiomeType BiomeType, int32 FormationCount = 3);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ScatterEnvironmentProps(FVector Center, float Radius, EJurassicBiomeType BiomeType, EEnvironmentDetailLevel DetailLevel);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void CreateGroundScatter(FVector Center, float Radius, EJurassicBiomeType BiomeType, float Density = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlaceWaterFeatures(FVector Center, float Radius, EJurassicBiomeType BiomeType);

    // Narrative environment storytelling
    UFUNCTION(BlueprintCallable, Category = "Narrative Environment")
    void CreateNarrativeCluster(FVector Location, const FNarrativeEnvironmentCluster& ClusterData);

    UFUNCTION(BlueprintCallable, Category = "Narrative Environment")
    void PlaceDinosaurActivitySigns(FVector Center, float Radius, class UClass* DinosaurClass);

    UFUNCTION(BlueprintCallable, Category = "Narrative Environment")
    void CreateAbandonedCampsite(FVector Location, float AgeInDays = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative Environment")
    void PlaceGemaClues(FVector Location, int32 ClueIntensity = 1);

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantDetails(FVector PlayerLocation, float CullDistance = 5000.0f);

    // Material and lighting integration
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyBiomeAtmosphere(FVector Center, float Radius, EJurassicBiomeType BiomeType);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    UJurassicEnvironmentProfile* GetEnvironmentProfile(EJurassicBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    bool IsLocationSuitableForProp(FVector Location, const FEnvironmentProp& PropData);

protected:
    // Environment profiles for each biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Profiles")
    TMap<EJurassicBiomeType, UJurassicEnvironmentProfile*> EnvironmentProfiles;

    // Component references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UJurassicVegetationSystem* VegetationSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UJurassicMaterialSystem* MaterialSystem;

    // Instanced mesh components for performance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Instancing")
    TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> InstancedMeshComponents;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPropsPerChunk = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ChunkSize = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseInstancedRendering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseOcclusionCulling = true;

    // Noise settings for natural distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float PropNoiseScale = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float ClusteringNoiseScale = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float StorytellingNoiseScale = 0.001f;

private:
    // Internal prop placement functions
    void PlaceEnvironmentProp(const FEnvironmentProp& PropData, FVector Location, FRotator Rotation, FVector Scale);
    void PlaceRockFormation(const FRockFormation& Formation, FVector Center);
    void PlaceNarrativeProp(const FEnvironmentProp& PropData, FVector Location, const FString& StoryContext);

    // Utility functions
    UHierarchicalInstancedStaticMeshComponent* GetOrCreateInstancedComponent(UStaticMesh* Mesh);
    float GetEnvironmentalNoise(FVector Location, float Scale);
    FVector GetRandomLocationInRadius(FVector Center, float Radius);
    bool CheckPropPlacementRules(FVector Location, const FEnvironmentProp& PropData);
    float CalculateDetailLevelMultiplier(EEnvironmentDetailLevel DetailLevel);

    // Storytelling helpers
    void CreateDinosaurNest(FVector Location, float NestSize = 300.0f);
    void CreateDinosaurFeedingSite(FVector Location, class UClass* PreyClass = nullptr);
    void CreateDinosaurBattleSite(FVector Location, class UClass* Predator = nullptr, class UClass* Prey = nullptr);
    void CreateWateringHole(FVector Location, float Size = 500.0f);
    void CreateMigrationPath(FVector Start, FVector End, float PathWidth = 200.0f);

    // Performance tracking
    UPROPERTY()
    int32 CurrentPropCount = 0;

    UPROPERTY()
    TArray<FVector> PlacedPropLocations;

    UPROPERTY()
    TMap<FString, int32> StoryClusterCounts;
};