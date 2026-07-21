#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_PrehistoricArchitecture.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneCircle         UMETA(DisplayName = "Stone Circle"),
    MegalithDolmen      UMETA(DisplayName = "Megalith Dolmen"),
    CaveEntrance        UMETA(DisplayName = "Cave Entrance"),
    RockFormation       UMETA(DisplayName = "Rock Formation"),
    AncientRuins        UMETA(DisplayName = "Ancient Ruins")
};

UENUM(BlueprintType)
enum class EArch_WeatheringLevel : uint8
{
    Fresh               UMETA(DisplayName = "Fresh Stone"),
    Weathered           UMETA(DisplayName = "Weathered"),
    HeavilyWeathered    UMETA(DisplayName = "Heavily Weathered"),
    Ancient             UMETA(DisplayName = "Ancient"),
    Crumbling           UMETA(DisplayName = "Crumbling")
};

USTRUCT(BlueprintType)
struct FArch_StructureProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StoneCircle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_WeatheringLevel WeatheringLevel = EArch_WeatheringLevel::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 StoneCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float CircleRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasLichenGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float MossIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float LichenIntensity = 0.4f;

    FArch_StructureProperties()
    {
        StructureType = EArch_StructureType::StoneCircle;
        WeatheringLevel = EArch_WeatheringLevel::Weathered;
        StructureScale = 1.0f;
        StoneCount = 8;
        CircleRadius = 500.0f;
        bHasMossGrowth = true;
        bHasLichenGrowth = true;
        MossIntensity = 0.6f;
        LichenIntensity = 0.4f;
    }
};

USTRUCT(BlueprintType)
struct FArch_InteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bIsInteractable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionPrompt = TEXT("Examine ancient structure");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bProvidesWeatherShelter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float ShelterRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float WeatherProtection = 0.3f;

    FArch_InteractionData()
    {
        bIsInteractable = true;
        InteractionPrompt = TEXT("Examine ancient structure");
        InteractionRange = 300.0f;
        bProvidesWeatherShelter = false;
        ShelterRadius = 200.0f;
        WeatherProtection = 0.3f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AArch_PrehistoricArchitecture : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricArchitecture();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* InteractionTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UStaticMeshComponent*> StoneMeshComponents;

    // Structure Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureProperties StructureProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_InteractionData InteractionData;

    // Visual Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* StandingStoneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* DolmenCapMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* RuinBlockMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UMaterialInterface* MossyStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UMaterialInterface* WeatheredStoneMaterial;

    // Structure Generation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureAppearance();

    // Specific Structure Types
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateStoneCircle();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateMegalithDolmen();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateRockFormation();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateAncientRuins();

    // Material Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    class UMaterialInterface* GetMaterialForWeathering(EArch_WeatheringLevel WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringToStones();

    // Interaction System
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnPlayerEnterRange();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnPlayerExitRange();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnPlayerInteract();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsPlayerInShelterRange(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    float GetWeatherProtectionAtLocation(const FVector& Location) const;

    // Environmental Integration
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalEffects();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SpawnMossAndLichen();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateSeasonalAppearance(float SeasonProgress);

protected:
    // Internal State
    UPROPERTY()
    bool bIsPlayerInRange = false;

    UPROPERTY()
    float LastInteractionTime = 0.0f;

    UPROPERTY()
    TArray<FVector> StonePositions;

    UPROPERTY()
    TArray<FRotator> StoneRotations;

    // Helper Functions
    void InitializeComponents();
    void SetupInteractionTrigger();
    FVector CalculateStonePosition(int32 StoneIndex, float Radius, int32 TotalStones);
    FRotator CalculateStoneRotation(int32 StoneIndex, const FVector& Position);
    void RandomizeStoneTransforms();
};