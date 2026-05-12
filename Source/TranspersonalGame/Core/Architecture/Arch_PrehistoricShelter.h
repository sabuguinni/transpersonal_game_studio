#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "Arch_PrehistoricShelter.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveDwelling        UMETA(DisplayName = "Cave Dwelling"),
    TreePlatform        UMETA(DisplayName = "Tree Platform"),
    RockShelter         UMETA(DisplayName = "Rock Shelter"),
    LeanToShelter       UMETA(DisplayName = "Lean-To Shelter"),
    BurrowShelter       UMETA(DisplayName = "Burrow Shelter"),
    TemporaryWindbreak  UMETA(DisplayName = "Temporary Windbreak")
};

UENUM(BlueprintType)
enum class EArch_ShelterCondition : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    WellMaintained  UMETA(DisplayName = "Well Maintained"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Abandoned       UMETA(DisplayName = "Abandoned"),
    Ruined          UMETA(DisplayName = "Ruined")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    EArch_ShelterType ShelterType = EArch_ShelterType::LeanToShelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    EArch_ShelterCondition Condition = EArch_ShelterCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float WeatherProtection = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float ThermalInsulation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    bool bIsConcealed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float ConstructionTime = 120.0f; // minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Properties")
    float DurabilityDays = 30.0f;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::LeanToShelter;
        Condition = EArch_ShelterCondition::WellMaintained;
        WeatherProtection = 0.7f;
        ThermalInsulation = 0.5f;
        MaxOccupants = 2;
        bHasFirePit = false;
        bHasStorage = false;
        bIsConcealed = false;
        ConstructionTime = 120.0f;
        DurabilityDays = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    TArray<FVector> SleepingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    FVector FirePitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    TArray<FVector> StorageLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    TArray<FVector> ToolRackLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    FVector EntranceLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    float FloorArea = 16.0f; // square meters

    FArch_InteriorLayout()
    {
        SleepingAreas.Empty();
        FirePitLocation = FVector::ZeroVector;
        StorageLocations.Empty();
        ToolRackLocations.Empty();
        EntranceLocation = FVector::ZeroVector;
        FloorArea = 16.0f;
    }
};

/**
 * Prehistoric shelter actor representing various types of primitive human dwellings
 * Handles construction, maintenance, and environmental interaction of prehistoric shelters
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrehistoricShelter : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricShelter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    // Shelter Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterProperties ShelterProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_InteriorLayout InteriorLayout;

    // Environmental Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float CurrentWeatherExposure = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float CurrentTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float DeteriorationRate = 1.0f;

    // Occupancy
    UPROPERTY(BlueprintReadOnly, Category = "Occupancy")
    int32 CurrentOccupants = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Occupancy")
    TArray<AActor*> OccupantActors;

    // Functionality
    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    bool CanEnterShelter(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    bool EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    bool ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    void DeteriorateShelter(float DeteriorationAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    float GetEffectiveWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    float GetEffectiveThermalInsulation() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    void UpdateShelterCondition();

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter Operations")
    void GenerateInteriorLayout();

    // Environmental Response
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void OnWeatherChange(float WeatherIntensity, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void OnSeasonalChange(float SeasonalFactor);

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool PlaceInteriorItem(AActor* Item, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool RemoveInteriorItem(AActor* Item);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    TArray<FVector> GetAvailableStorageLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    FVector GetBestSleepingLocation() const;

    // Construction System
    UFUNCTION(BlueprintCallable, Category = "Construction")
    static bool CanConstructShelterAt(FVector Location, EArch_ShelterType ShelterType);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    static TArray<FString> GetRequiredMaterials(EArch_ShelterType ShelterType);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    static float GetConstructionTime(EArch_ShelterType ShelterType);

protected:
    // Internal state management
    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    float AccumulatedDeteriorationTime = 0.0f;

    // Helper functions
    void InitializeShelterMesh();
    void UpdateShelterAppearance();
    void ProcessEnvironmentalEffects(float DeltaTime);
    void ValidateOccupancy();
    FVector CalculateOptimalFirePitLocation() const;
    TArray<FVector> CalculateOptimalStorageLocations() const;
    void ApplyConditionToMesh();
};