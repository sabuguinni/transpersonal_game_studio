#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Arch_CretaceousLandmarkSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_LandmarkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FString LandmarkName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    EBiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    bool bIsNavigationLandmark;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    float VisibilityRange;

    FArch_LandmarkData()
    {
        LandmarkName = TEXT("Unknown Landmark");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = 1.0f;
        AssociatedBiome = EBiomeType::Temperate;
        bIsNavigationLandmark = true;
        VisibilityRange = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralIntegrityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    float StructuralHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    float WeatherResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    float AgeInYears;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    bool bRequiresMaintenance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    TArray<FString> DamageTypes;

    FArch_StructuralIntegrityData()
    {
        StructuralHealth = 100.0f;
        WeatherResistance = 75.0f;
        AgeInYears = 0.0f;
        bRequiresMaintenance = false;
        DamageTypes.Empty();
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_CretaceousLandmarkSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_CretaceousLandmarkSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Landmark Management
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Landmarks")
    void InitializeLandmarkSystem();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Landmarks")
    void SpawnLandmarkAtLocation(const FArch_LandmarkData& LandmarkData);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Landmarks")
    void UpdateLandmarkIntegrity(const FString& LandmarkName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Landmarks")
    TArray<FArch_LandmarkData> GetLandmarksInRange(const FVector& PlayerLocation, float Range);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Landmarks")
    void ApplyWeatherEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Landmarks")
    bool IsLandmarkVisible(const FArch_LandmarkData& Landmark, const FVector& ViewerLocation);

    // Structural Integrity
    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void UpdateStructuralIntegrity(const FString& LandmarkName, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    FArch_StructuralIntegrityData GetStructuralData(const FString& LandmarkName);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void RepairLandmark(const FString& LandmarkName, float RepairAmount);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark System")
    TArray<FArch_LandmarkData> ActiveLandmarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark System")
    TMap<FString, FArch_StructuralIntegrityData> StructuralIntegrityMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark System")
    float WeatherEffectIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark System")
    float MaxLandmarkDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark System")
    bool bEnableWeatherEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark System")
    TArray<UStaticMesh*> LandmarkMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark System")
    TArray<UMaterialInterface*> WeatheredMaterials;

private:
    void ProcessLandmarkAging(float DeltaTime);
    void UpdateVisualEffects(const FString& LandmarkName);
    AStaticMeshActor* CreateLandmarkActor(const FArch_LandmarkData& LandmarkData);
    void ApplyMaterialBasedOnIntegrity(AStaticMeshActor* LandmarkActor, float IntegrityPercent);
};