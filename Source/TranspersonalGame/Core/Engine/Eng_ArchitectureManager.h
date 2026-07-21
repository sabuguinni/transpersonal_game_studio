#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Eng_ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BuildingType : uint8
{
    None            UMETA(DisplayName = "None"),
    Shelter         UMETA(DisplayName = "Basic Shelter"),
    Storage         UMETA(DisplayName = "Storage Hut"),
    Crafting        UMETA(DisplayName = "Crafting Station"),
    Firepit         UMETA(DisplayName = "Fire Pit"),
    Watchtower      UMETA(DisplayName = "Watch Tower"),
    Fence           UMETA(DisplayName = "Wooden Fence"),
    Bridge          UMETA(DisplayName = "Rope Bridge")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BuildingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EEng_BuildingType BuildingType = EEng_BuildingType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bIsConstructed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FString> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float ConstructionProgress = 0.0f;

    FEng_BuildingData()
    {
        BuildingType = EEng_BuildingType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Health = 100.0f;
        bIsConstructed = false;
        ConstructionProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxBuildings = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float BuildingSnapDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableAutoRepair = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatherDamageRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bRequireMaterials = true;

    FEng_ArchitectureSettings()
    {
        MaxBuildings = 50;
        BuildingSnapDistance = 100.0f;
        bEnableAutoRepair = true;
        WeatherDamageRate = 1.0f;
        bRequireMaterials = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Building Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CreateBuilding(EEng_BuildingType BuildingType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RemoveBuilding(int32 BuildingIndex);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateBuildingConstruction(int32 BuildingIndex, float ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_BuildingData> GetAllBuildings() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_BuildingData GetBuildingData(int32 BuildingIndex) const;

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsValidBuildingLocation(FVector Location, EEng_BuildingType BuildingType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetNearestBuildingDistance(FVector Location) const;

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetArchitectureSettings(const FEng_ArchitectureSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_ArchitectureSettings GetArchitectureSettings() const;

    // Weather damage system
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherDamage(float DeltaTime);

    // Material requirements
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetRequiredMaterials(EEng_BuildingType BuildingType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool HasRequiredMaterials(EEng_BuildingType BuildingType) const;

    // Debug and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void DebugSpawnTestBuildings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ClearAllBuildings();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_BuildingData> Buildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_ArchitectureSettings Settings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    bool bIsInitialized = false;

private:
    void InitializeDefaultSettings();
    FString GetBuildingMeshPath(EEng_BuildingType BuildingType) const;
    AActor* SpawnBuildingActor(const FEng_BuildingData& BuildingData);
};