#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    CaveDwelling        UMETA(DisplayName = "Cave Dwelling"),
    ElevatedPlatform    UMETA(DisplayName = "Elevated Platform"),
    GroundShelter       UMETA(DisplayName = "Ground Shelter"),
    DefensiveOutpost    UMETA(DisplayName = "Defensive Outpost"),
    StoragePit          UMETA(DisplayName = "Storage Pit")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone               UMETA(DisplayName = "Stone"),
    Wood                UMETA(DisplayName = "Wood"),
    AnimalHide          UMETA(DisplayName = "Animal Hide"),
    Bone                UMETA(DisplayName = "Bone"),
    Clay                UMETA(DisplayName = "Clay")
};

USTRUCT(BlueprintType)
struct FArch_DwellingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingType DwellingType = EArch_DwellingType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FVector Dimensions = FVector(400.0f, 300.0f, 250.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_MaterialType PrimaryMaterial = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float DefenseRating = 0.5f;

    FArch_DwellingConfig()
    {
        DwellingType = EArch_DwellingType::CaveDwelling;
        Dimensions = FVector(400.0f, 300.0f, 250.0f);
        PrimaryMaterial = EArch_MaterialType::Stone;
        MaxOccupants = 4;
        bHasFirePit = true;
        bHasStorage = true;
        DefenseRating = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FArch_InteriorElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_MaterialType Material = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsEssential = false;

    FArch_InteriorElement()
    {
        ElementName = TEXT("GenericElement");
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Material = EArch_MaterialType::Stone;
        bIsEssential = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitectureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dwelling Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CreateDwelling(const FArch_DwellingConfig& Config, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveDwelling(int32 DwellingID);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<int32> GetNearbyDwellings(const FVector& Location, float Radius) const;

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool AddInteriorElement(int32 DwellingID, const FArch_InteriorElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveInteriorElement(int32 DwellingID, const FString& ElementName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_InteriorElement> GetInteriorElements(int32 DwellingID) const;

    // Settlement Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CreateSettlement(const FVector& CenterLocation, float Radius, int32 DwellingCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateSettlementDefenses(int32 SettlementID, float DefenseMultiplier);

    // Material and Weathering
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(int32 DwellingID, float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateMaterialCondition(int32 DwellingID, float DeltaTime);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float CalculateDwellingCapacity(const FArch_DwellingConfig& Config) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsDwellingOccupied(int32 DwellingID) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetOptimalDwellingLocation(const FVector& SearchCenter, float SearchRadius, EArch_DwellingType DwellingType) const;

protected:
    // Internal data structures
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<int32, FArch_DwellingConfig> ActiveDwellings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<int32, TArray<FArch_InteriorElement>> DwellingInteriors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<int32, FVector> DwellingLocations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<int32, TArray<int32>> Settlements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxWeatheringAmount = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxDwellings = 50;

private:
    int32 NextDwellingID = 1;
    int32 NextSettlementID = 1;

    // Helper functions
    bool ValidateDwellingLocation(const FVector& Location, const FArch_DwellingConfig& Config) const;
    void SpawnDwellingMesh(const FArch_DwellingConfig& Config, const FVector& Location, const FRotator& Rotation);
    void CreateDefaultInteriorElements(int32 DwellingID, const FArch_DwellingConfig& Config);
};