#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Arch_StructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneArchway    UMETA(DisplayName = "Stone Archway"),
    TribalHut       UMETA(DisplayName = "Tribal Hut"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin"),
    WoodenPlatform  UMETA(DisplayName = "Wooden Platform"),
    RockShelter     UMETA(DisplayName = "Rock Shelter")
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
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StoneArchway;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_WeatheringLevel WeatheringLevel = EArch_WeatheringLevel::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName = TEXT("Unknown Structure");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float AgeInYears = 500.0f;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StoneArchway;
        WeatheringLevel = EArch_WeatheringLevel::Weathered;
        StructuralIntegrity = 100.0f;
        bHasMossGrowth = true;
        bIsHabitable = false;
        AgeInYears = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DetailMesh;

    // Structure Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    EBiomeType AssociatedBiome = EBiomeType::Savana;

    // Weathering System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float MossGrowthRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    bool bEnableWeathering = true;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(EArch_StructureType Type, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetWeatheringLevel(EArch_WeatheringLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    FString GetStructureDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool CanPlayerEnter() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void RepairStructure(float RepairAmount);

private:
    void UpdateMeshBasedOnType();
    void UpdateMaterialsBasedOnWeathering();
    void ApplyBiomeSpecificEffects();

    float WeatheringTimer = 0.0f;
    float MossGrowthTimer = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_StructureManager : public UObject
{
    GENERATED_BODY()

public:
    UArch_StructureManager();

    // Structure Management
    UFUNCTION(BlueprintCallable, Category = "Structure Manager")
    static AArch_StructureActor* SpawnStructureAtLocation(UWorld* World, EArch_StructureType Type, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Structure Manager")
    static void PopulateBiomeWithStructures(UWorld* World, EBiomeType Biome, int32 StructureCount = 10);

    UFUNCTION(BlueprintCallable, Category = "Structure Manager")
    static TArray<AArch_StructureActor*> GetAllStructuresInBiome(UWorld* World, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Structure Manager")
    static void ApplyWeatheringToAllStructures(UWorld* World, float WeatheringMultiplier = 1.0f);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Structure Manager")
    static FVector GetBiomeCoordinates(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Structure Manager")
    static EArch_StructureType GetRandomStructureTypeForBiome(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Structure Manager")
    static FString GetStructureTypeName(EArch_StructureType Type);

private:
    static TMap<EBiomeType, TArray<EArch_StructureType>> BiomeStructureMap;
    static void InitializeBiomeStructureMap();
};