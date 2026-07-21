#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Arch_PrehistoricDwellingSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    CaveNatural,
    CaveModified,
    StoneShelter,
    WoodFrameShelter,
    LeafHut,
    RockOverhang,
    BurrowDwelling,
    TreeHollowShelter
};

UENUM(BlueprintType)
enum class EArch_DwellingCondition : uint8
{
    Pristine,
    WellMaintained,
    Weathered,
    Damaged,
    Abandoned,
    Ruined,
    Collapsed
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DwellingConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    EArch_DwellingType DwellingType = EArch_DwellingType::CaveNatural;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    EArch_DwellingCondition Condition = EArch_DwellingCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    float InteriorSpace = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    bool bHasToolStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    bool bHasSleepingArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    bool bHasFoodStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    float StructuralIntegrity = 1.0f;

    FArch_DwellingConfiguration()
    {
        DwellingType = EArch_DwellingType::CaveNatural;
        Condition = EArch_DwellingCondition::WellMaintained;
        InteriorSpace = 50.0f;
        MaxOccupants = 4;
        bHasFirePit = true;
        bHasToolStorage = true;
        bHasSleepingArea = true;
        bHasFoodStorage = false;
        WeatherProtection = 0.8f;
        StructuralIntegrity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FVector RelativeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FString MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    bool bIsEssential = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    float DurabilityFactor = 1.0f;

    FArch_InteriorElement()
    {
        ElementName = TEXT("GenericElement");
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        MeshPath = TEXT("");
        bIsEssential = false;
        DurabilityFactor = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrehistoricDwelling : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricDwelling();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FArch_DwellingConfiguration DwellingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FArch_InteriorElement> InteriorElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbientTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float LightLevel = 0.3f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void InitializeDwelling(const FArch_DwellingConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void AddInteriorElement(const FArch_InteriorElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void RemoveInteriorElement(const FString& ElementName);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    float GetComfortLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void UpdateStructuralIntegrity(float DeltaIntegrity);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void ApplyWeatherDamage(float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void RepairDwelling(float RepairAmount);

    UFUNCTION(BlueprintPure, Category = "Dwelling")
    EArch_DwellingType GetDwellingType() const { return DwellingConfig.DwellingType; }

    UFUNCTION(BlueprintPure, Category = "Dwelling")
    EArch_DwellingCondition GetDwellingCondition() const { return DwellingConfig.Condition; }

    UFUNCTION(BlueprintPure, Category = "Dwelling")
    float GetWeatherProtection() const { return DwellingConfig.WeatherProtection; }

private:
    void UpdateDwellingAppearance();
    void SpawnInteriorElements();
    void UpdateEnvironmentalFactors();
    UStaticMesh* GetMeshForDwellingType(EArch_DwellingType Type) const;
};

UCLASS()
class TRANSPERSONALGAME_API UArch_PrehistoricDwellingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    AArch_PrehistoricDwelling* CreateDwelling(const FVector& Location, const FArch_DwellingConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void RegisterDwelling(AArch_PrehistoricDwelling* Dwelling);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void UnregisterDwelling(AArch_PrehistoricDwelling* Dwelling);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    TArray<AArch_PrehistoricDwelling*> GetDwellingsInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    AArch_PrehistoricDwelling* FindNearestDwelling(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    void UpdateAllDwellings(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dwelling System")
    int32 GetTotalDwellingCount() const { return RegisteredDwellings.Num(); }

protected:
    UPROPERTY()
    TArray<AArch_PrehistoricDwelling*> RegisteredDwellings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Config")
    float MaxDwellingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Config")
    int32 MaxDwellings = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Config")
    float WeatherUpdateInterval = 30.0f;

private:
    float LastWeatherUpdate = 0.0f;
    void ProcessWeatherEffects();
    void CleanupInvalidDwellings();
};