#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Arch_InteriorHabitationSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_InteriorType : uint8
{
    CaveInterior        UMETA(DisplayName = "Cave Interior"),
    ShelterInterior     UMETA(DisplayName = "Shelter Interior"),
    CliffDwelling       UMETA(DisplayName = "Cliff Dwelling"),
    UndergroundChamber  UMETA(DisplayName = "Underground Chamber")
};

UENUM(BlueprintType)
enum class EArch_HabitationFeature : uint8
{
    FirePit             UMETA(DisplayName = "Fire Pit"),
    SleepingArea        UMETA(DisplayName = "Sleeping Area"),
    StorageNiche        UMETA(DisplayName = "Storage Niche"),
    ToolRack            UMETA(DisplayName = "Tool Rack"),
    WaterContainer      UMETA(DisplayName = "Water Container"),
    FoodPreparation     UMETA(DisplayName = "Food Preparation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Feature")
    EArch_HabitationFeature FeatureType = EArch_HabitationFeature::FirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Feature")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Feature")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Feature")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Feature")
    float UsageIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Feature")
    int32 LastUsedDaysAgo = 0;

    FArch_InteriorFeature()
    {
        FeatureType = EArch_HabitationFeature::FirePit;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        bIsActive = true;
        UsageIntensity = 1.0f;
        LastUsedDaysAgo = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_HabitationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    EArch_InteriorType InteriorType = EArch_InteriorType::CaveInterior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    TArray<FArch_InteriorFeature> InteriorFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    int32 MaxOccupants = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    float ComfortLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    float HygieneFactor = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    bool bHasFireSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    bool bHasWaterAccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation")
    float AbandonmentLevel = 0.0f;

    FArch_HabitationData()
    {
        InteriorType = EArch_InteriorType::CaveInterior;
        InteriorFeatures.Empty();
        MaxOccupants = 6;
        ComfortLevel = 0.5f;
        HygieneFactor = 0.3f;
        bHasFireSource = false;
        bHasWaterAccess = false;
        AbandonmentLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorHabitationSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorHabitationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* HabitationVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* AmbientLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation Data")
    FArch_HabitationData HabitationInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation Data")
    TArray<AActor*> CurrentInhabitants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitation Data")
    TArray<UStaticMeshComponent*> FeatureMeshes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    void InitializeInteriorFeatures();

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    bool AddInteriorFeature(EArch_HabitationFeature FeatureType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    bool RemoveInteriorFeature(EArch_HabitationFeature FeatureType);

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    void UseFeature(EArch_HabitationFeature FeatureType, AActor* User);

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    float GetComfortLevel() const { return HabitationInfo.ComfortLevel; }

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    float GetHygieneFactor() const { return HabitationInfo.HygieneFactor; }

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    bool HasFireSource() const { return HabitationInfo.bHasFireSource; }

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    bool HasWaterAccess() const { return HabitationInfo.bHasWaterAccess; }

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    int32 GetAvailableSpace() const;

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    void SimulateAbandonment(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Habitation")
    void UpdateInteriorConditions();

protected:
    UFUNCTION()
    void OnHabitationVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnHabitationVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Habitation")
    void OnInhabitantEntered(AActor* Inhabitant);

    UFUNCTION(BlueprintImplementableEvent, Category = "Habitation")
    void OnInhabitantExited(AActor* Inhabitant);

    UFUNCTION(BlueprintImplementableEvent, Category = "Habitation")
    void OnFeatureUsed(EArch_HabitationFeature FeatureType, AActor* User);

    void CreateFeatureMesh(const FArch_InteriorFeature& Feature);
    void UpdateFeatureVisuals();
    void CalculateComfortLevel();
    void ProcessAbandonmentEffects(float DeltaTime);
};