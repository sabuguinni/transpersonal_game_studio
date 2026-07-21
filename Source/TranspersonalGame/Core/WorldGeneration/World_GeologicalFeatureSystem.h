#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_GeologicalFeatureSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_GeologicalFeatureType : uint8
{
    Volcano         UMETA(DisplayName = "Volcanic Formation"),
    Canyon          UMETA(DisplayName = "Canyon System"),
    Mesa            UMETA(DisplayName = "Mesa/Plateau"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    MineralDeposit  UMETA(DisplayName = "Mineral Deposit"),
    NaturalLandmark UMETA(DisplayName = "Natural Landmark"),
    RiverSegment    UMETA(DisplayName = "River Segment"),
    TransitionZone  UMETA(DisplayName = "Biome Transition")
};

UENUM(BlueprintType)
enum class EWorld_MineralType : uint8
{
    Iron        UMETA(DisplayName = "Iron Ore"),
    Copper      UMETA(DisplayName = "Copper Ore"),
    Stone       UMETA(DisplayName = "Stone Quarry"),
    Clay        UMETA(DisplayName = "Clay Deposit"),
    Flint       UMETA(DisplayName = "Flint Outcrop"),
    Obsidian    UMETA(DisplayName = "Obsidian Formation"),
    Limestone   UMETA(DisplayName = "Limestone Deposit")
};

UENUM(BlueprintType)
enum class EWorld_RiverType : uint8
{
    MountainStream  UMETA(DisplayName = "Mountain Stream"),
    Waterfall       UMETA(DisplayName = "Waterfall"),
    Rapids          UMETA(DisplayName = "Rapids"),
    MeanderingRiver UMETA(DisplayName = "Meandering River"),
    WideRiver       UMETA(DisplayName = "Wide River"),
    Delta           UMETA(DisplayName = "River Delta")
};

USTRUCT(BlueprintType)
struct FWorld_GeologicalFeatureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    EWorld_GeologicalFeatureType FeatureType = EWorld_GeologicalFeatureType::NaturalLandmark;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    FString FeatureName = TEXT("UnnamedFeature");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    float InfluenceRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature")
    bool bIsActive = true;

    FWorld_GeologicalFeatureData()
    {
        FeatureType = EWorld_GeologicalFeatureType::NaturalLandmark;
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        Rotation = FRotator::ZeroRotator;
        FeatureName = TEXT("UnnamedFeature");
        InfluenceRadius = 500.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FWorld_MineralDepositData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
    EWorld_MineralType MineralType = EWorld_MineralType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
    float Richness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
    float RemainingQuantity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
    bool bIsAccessible = true;

    FWorld_MineralDepositData()
    {
        MineralType = EWorld_MineralType::Stone;
        Location = FVector::ZeroVector;
        Richness = 1.0f;
        RemainingQuantity = 100.0f;
        bIsAccessible = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_GeologicalFeatureSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_GeologicalFeatureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FeatureMeshComponent;

    // Feature Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Feature")
    FWorld_GeologicalFeatureData FeatureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Feature")
    TArray<FWorld_MineralDepositData> MineralDeposits;

    // Volcanic System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic")
    bool bIsVolcanicFeature = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic", meta = (EditCondition = "bIsVolcanicFeature"))
    float VolcanicActivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic", meta = (EditCondition = "bIsVolcanicFeature"))
    bool bIsActiveVolcano = false;

    // Canyon System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon")
    bool bIsCanyonFeature = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon", meta = (EditCondition = "bIsCanyonFeature"))
    float CanyonDepth = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Canyon", meta = (EditCondition = "bIsCanyonFeature"))
    float CanyonWidth = 200.0f;

    // Cave System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    bool bIsCaveEntrance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave", meta = (EditCondition = "bIsCaveEntrance"))
    float CaveDepth = -200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave", meta = (EditCondition = "bIsCaveEntrance"))
    int32 ChamberCount = 3;

    // River System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    bool bIsRiverFeature = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River", meta = (EditCondition = "bIsRiverFeature"))
    EWorld_RiverType RiverType = EWorld_RiverType::MeanderingRiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River", meta = (EditCondition = "bIsRiverFeature"))
    float WaterFlow = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    // Feature Management
    UFUNCTION(BlueprintCallable, Category = "Geological Feature")
    void InitializeFeature(const FWorld_GeologicalFeatureData& NewFeatureData);

    UFUNCTION(BlueprintCallable, Category = "Geological Feature")
    void UpdateFeatureScale(const FVector& NewScale);

    UFUNCTION(BlueprintCallable, Category = "Geological Feature")
    void SetFeatureActive(bool bActive);

    // Mineral System
    UFUNCTION(BlueprintCallable, Category = "Mineral")
    void AddMineralDeposit(const FWorld_MineralDepositData& NewDeposit);

    UFUNCTION(BlueprintCallable, Category = "Mineral")
    bool ExtractMinerals(EWorld_MineralType MineralType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Mineral")
    float GetMineralQuantity(EWorld_MineralType MineralType) const;

    // Volcanic System
    UFUNCTION(BlueprintCallable, Category = "Volcanic")
    void SetVolcanicActivity(float Activity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic")
    void TriggerVolcanicEruption();

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetEnvironmentalInfluence(const FVector& TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    TArray<FWorld_GeologicalFeatureData> GetNearbyFeatures(const FVector& Location, float SearchRadius) const;

    // Terrain Morphing
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void ApplyTerrainMorphing(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetTerrainHeightAt(const FVector& WorldLocation) const;

private:
    // Internal feature management
    void UpdateVolcanicEffects(float DeltaTime);
    void UpdateRiverFlow(float DeltaTime);
    void UpdateCaveSystem(float DeltaTime);
    void ProcessMineralDepletion(float DeltaTime);

    // Cached references
    UPROPERTY()
    TArray<AActor*> NearbyActors;

    // Performance optimization
    float LastUpdateTime = 0.0f;
    float UpdateInterval = 1.0f;
};