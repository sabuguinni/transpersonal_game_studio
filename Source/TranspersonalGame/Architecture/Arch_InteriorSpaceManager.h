#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_InteriorSpaceManager.generated.h"

UENUM(BlueprintType)
enum class EArch_InteriorType : uint8
{
    Dwelling,
    Storage,
    Workshop,
    Shelter,
    Sacred,
    Communal
};

UENUM(BlueprintType)
enum class EArch_InteriorMood : uint8
{
    Cozy,
    Abandoned,
    Active,
    Mysterious,
    Dangerous,
    Peaceful
};

USTRUCT(BlueprintType)
struct FArch_InteriorProps
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FVector> FirePitLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FVector> SleepingAreaLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FVector> StorageLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FVector> ToolLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    bool bHasSleepingArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    float LightingIntensity = 1.0f;

    FArch_InteriorProps()
    {
        bHasFirePit = false;
        bHasSleepingArea = false;
        bHasStorage = false;
        LightingIntensity = 1.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AArch_InteriorSpaceManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorSpaceManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorType InteriorType = EArch_InteriorType::Dwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorMood InteriorMood = EArch_InteriorMood::Cozy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FArch_InteriorProps InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float InteriorTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float AirQuality = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    int32 MaxOccupants = 4;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    TArray<AActor*> CurrentOccupants;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetupInterior(EArch_InteriorType Type, EArch_InteriorMood Mood);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SpawnInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool CanEnterInterior(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void OnActorEnterInterior(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void OnActorExitInterior(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    FVector GetRandomInteriorLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    TArray<FVector> GetPropLocations(const FString& PropType) const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    float GetComfortLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void UpdateEnvironmentalEffects();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnInteriorEntered(AActor* Actor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnInteriorExited(AActor* Actor);

private:
    void CreateInteriorLayout();
    void SpawnFirePit(const FVector& Location);
    void SpawnSleepingArea(const FVector& Location);
    void SpawnStorage(const FVector& Location);
    void SpawnTools(const FVector& Location);
    void UpdateLighting();
    void UpdateTemperature();

    UPROPERTY()
    TArray<AActor*> SpawnedProps;

    float EnvironmentUpdateTimer = 0.0f;
};