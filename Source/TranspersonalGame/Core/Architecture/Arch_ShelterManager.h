#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageArea = true;

    FArch_ShelterConfig()
    {
        ShelterType = EArch_ShelterType::CaveEntrance;
        ProtectionRadius = 500.0f;
        WeatherProtection = 0.8f;
        TemperatureModifier = 5.0f;
        MaxOccupants = 4;
        bHasFirePit = true;
        bHasStorageArea = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ProtectionZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StorageAreaMesh;

    // Shelter Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterConfig ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<UStaticMesh*> ShelterMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<UMaterialInterface*> ShelterMaterials;

    // Shelter State
    UPROPERTY(BlueprintReadOnly, Category = "Shelter State")
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter State")
    bool bIsOccupied;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter State")
    float CurrentTemperature;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter State")
    bool bFireLit;

    // Shelter Functions
    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(const FArch_ShelterConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void LightFire();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtectionAt(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureModifierAt(const FVector& Location);

    // Environmental Integration
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateShelterConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyWeatherEffects(float RainIntensity, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateFireEffects(float DeltaTime);

protected:
    // Internal Functions
    void SetupShelterMesh();
    void SetupProtectionZone();
    void SetupInteriorElements();
    void UpdateOccupancyState();
    bool IsLocationProtected(const FVector& Location);

    // Overlap Events
    UFUNCTION()
    void OnProtectionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProtectionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // Internal State
    float FireIntensity;
    float ShelterAge;
    float LastMaintenanceTime;
    bool bNeedsRepair;
    
    // Timers
    FTimerHandle FireUpdateTimer;
    FTimerHandle ConditionUpdateTimer;
};