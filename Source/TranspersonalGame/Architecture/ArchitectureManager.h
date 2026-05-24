#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave            UMETA(DisplayName = "Cave Dwelling"),
    ElevatedPlatform UMETA(DisplayName = "Elevated Platform"),
    StoneDwelling   UMETA(DisplayName = "Stone Dwelling"),
    UndergroundBunker UMETA(DisplayName = "Underground Bunker"),
    TemporaryLean   UMETA(DisplayName = "Temporary Lean-to")
};

UENUM(BlueprintType)
enum class EArch_DefenseLevel : uint8
{
    None        UMETA(DisplayName = "No Defense"),
    Basic       UMETA(DisplayName = "Basic Walls"),
    Reinforced  UMETA(DisplayName = "Reinforced Structure"),
    Fortified   UMETA(DisplayName = "Fortified Compound")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_DefenseLevel DefenseLevel = EArch_DefenseLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 Capacity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 100.0f;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::Cave;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        DefenseLevel = EArch_DefenseLevel::None;
        bHasFirePit = false;
        bHasStorage = false;
        Capacity = 1;
        StructuralIntegrity = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorComponent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString ComponentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float Durability = 100.0f;

    FArch_InteriorComponent()
    {
        ComponentName = TEXT("Unknown Component");
        RelativeLocation = FVector::ZeroVector;
        bIsActive = true;
        Durability = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Shelter Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CreateShelter(const FArch_ShelterData& ShelterData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RemoveShelter(int32 ShelterIndex);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_ShelterData> GetAllShelters() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_ShelterData GetNearestShelter(const FVector& Location, float MaxDistance = 5000.0f) const;

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool AddInteriorComponent(int32 ShelterIndex, const FArch_InteriorComponent& Component);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RemoveInteriorComponent(int32 ShelterIndex, const FString& ComponentName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_InteriorComponent> GetInteriorComponents(int32 ShelterIndex) const;

    // Defense System
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UpgradeDefenses(int32 ShelterIndex, EArch_DefenseLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float CalculateDefenseRating(int32 ShelterIndex) const;

    // Structural Integrity
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RepairStructure(int32 ShelterIndex, float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyDinosaurDamage(int32 ShelterIndex, float DamageAmount);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsShelterSafe(int32 ShelterIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetShelterCapacity(int32 ShelterIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanUpgradeShelter(int32 ShelterIndex) const;

protected:
    // Data Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_ShelterData> ActiveShelters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<int32, TArray<FArch_InteriorComponent>> ShelterInteriors;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float WeatherDamageRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DinosaurDamageMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float RepairEfficiency = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxSheltersPerArea = 5;

private:
    // Internal Helper Functions
    bool ValidateShelterLocation(const FVector& Location) const;
    void InitializeDefaultShelters();
    void UpdateStructuralIntegrity(float DeltaTime);
    int32 FindNearestShelterIndex(const FVector& Location) const;
};