#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../Core/SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

// Forward declarations
class UStaticMesh;
class AActor;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin"),
    TribalHut       UMETA(DisplayName = "Tribal Hut"),
    SacredSite      UMETA(DisplayName = "Sacred Site")
};

UENUM(BlueprintType)
enum class EArch_WeatheringLevel : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    LightWear       UMETA(DisplayName = "Light Wear"),
    Weathered       UMETA(DisplayName = "Weathered"),
    HeavilyWorn     UMETA(DisplayName = "Heavily Worn"),
    Ruined          UMETA(DisplayName = "Ruined")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_WeatheringLevel WeatheringLevel = EArch_WeatheringLevel::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float RotationVariance = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bAddMossOvergrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bAddDebrisScatter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float InteriorLightIntensity = 0.3f;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::CaveDwelling;
        WeatheringLevel = EArch_WeatheringLevel::Weathered;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        RotationVariance = 45.0f;
        bAddMossOvergrowth = true;
        bAddDebrisScatter = true;
        InteriorLightIntensity = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_BiomeStructureSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Structures")
    TArray<EArch_StructureType> PreferredStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Structures")
    float StructureDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Structures")
    float MinDistanceBetweenStructures = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Structures")
    EArch_WeatheringLevel TypicalWeathering = EArch_WeatheringLevel::Weathered;

    FArch_BiomeStructureSet()
    {
        PreferredStructures = {EArch_StructureType::CaveDwelling, EArch_StructureType::RockShelter};
        StructureDensity = 0.1f;
        MinDistanceBetweenStructures = 5000.0f;
        TypicalWeathering = EArch_WeatheringLevel::Weathered;
    }
};

/**
 * Manages architectural structures in the prehistoric world
 * Creates cave dwellings, stone pillars, ancient ruins, and tribal settlements
 * Each structure tells a story of the civilization that built it
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Structure Generation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* SpawnStructureAtLocation(const FVector& Location, const FArch_StructureConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateBiomeWithStructures(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructureCluster(const FVector& CenterLocation, int32 StructureCount, float ClusterRadius);

    // Structure Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringEffects(AActor* StructureActor, EArch_WeatheringLevel WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddInteriorLighting(AActor* StructureActor, float LightIntensity);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ScatterDebrisAroundStructure(AActor* StructureActor, float ScatterRadius);

    // Biome-Specific Generation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateSavannaStructures(const FVector& BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateForestStructures(const FVector& BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateDesertStructures(const FVector& BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateMountainStructures(const FVector& BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateSwampStructures(const FVector& BiomeCenter);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_StructureConfig GetDefaultConfigForBiome(EBiomeType BiomeType, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "Architecture")
    void GenerateTestStructures();

    UFUNCTION(CallInEditor, Category = "Architecture")
    void ValidateStructurePlacements();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    TMap<EBiomeType, FArch_BiomeStructureSet> BiomeStructureConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    float GlobalStructureDensity = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    float MinStructureSpacing = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    bool bAutoGenerateOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    int32 MaxStructuresPerBiome = 50;

    // Runtime Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture State")
    TArray<AActor*> SpawnedStructures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture State")
    int32 TotalStructuresGenerated = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture State")
    float LastGenerationTime = 0.0f;

private:
    // Internal Methods
    void InitializeBiomeConfigs();
    AActor* CreateStructureActor(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation);
    void ApplyStructureMaterials(AActor* StructureActor, EArch_StructureType StructureType, EArch_WeatheringLevel WeatheringLevel);
    FVector FindSuitableLocationNear(const FVector& PreferredLocation, EArch_StructureType StructureType);
    bool CheckStructureSpacing(const FVector& Location);
};