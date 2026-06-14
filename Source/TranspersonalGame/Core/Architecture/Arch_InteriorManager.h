#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_InteriorManager.generated.h"

UENUM(BlueprintType)
enum class EArch_InteriorType : uint8
{
    Cave,
    Shelter,
    Dwelling,
    Storage,
    Workshop,
    Ritual,
    Burial
};

USTRUCT(BlueprintType)
struct FArch_InteriorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorType InteriorType = EArch_InteriorType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float AmbientTemperature = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float Humidity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float AirQuality = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasNaturalLight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFireSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> FurniturePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> StoragePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float ComfortLevel = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CeilingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* AmbientLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* FireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Data")
    FArch_InteriorData InteriorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Settings")
    float LightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Settings")
    FLinearColor AmbientLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Settings")
    FLinearColor FireLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<AActor*> InteriorObjects;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsOccupied = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentTemperature = 18.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void InitializeInterior(EArch_InteriorType Type, FVector Dimensions);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void AddInteriorObject(AActor* Object, FVector LocalPosition);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void RemoveInteriorObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetFireActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void UpdateAmbientConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    float GetComfortLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool IsWellVentilated() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    FVector GetRandomFurniturePosition() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    FVector GetRandomStoragePosition() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetOccupied(bool bOccupied);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnInteriorEntered(AActor* Occupant);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnInteriorExited(AActor* Occupant);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnFireLit();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnFireExtinguished();

protected:
    void ConfigureInteriorForType(EArch_InteriorType Type);
    void SetupLighting();
    void SetupAmbientAudio();
    void UpdateLighting(float DeltaTime);
    void UpdateTemperature(float DeltaTime);

private:
    float FireFlickerTime = 0.0f;
    float TemperatureUpdateTimer = 0.0f;
    const float TemperatureUpdateInterval = 2.0f;
    bool bFireIsActive = false;
};

#include "Arch_InteriorManager.generated.h"