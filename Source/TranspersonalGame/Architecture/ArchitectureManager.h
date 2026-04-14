#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Hut             UMETA(DisplayName = "Stone Age Hut"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    Megalith        UMETA(DisplayName = "Megalithic Structure"),
    CliffDwelling   UMETA(DisplayName = "Cliff Dwelling"),
    Palisade        UMETA(DisplayName = "Wooden Palisade"),
    FirePit         UMETA(DisplayName = "Central Fire Pit"),
    StoragePit      UMETA(DisplayName = "Storage Pit"),
    Watchtower      UMETA(DisplayName = "Watchtower"),
    Bridge          UMETA(DisplayName = "Primitive Bridge"),
    Shrine          UMETA(DisplayName = "Ritual Shrine")
};

UENUM(BlueprintType)
enum class EArch_StructureState : uint8
{
    Intact          UMETA(DisplayName = "Intact"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Abandoned       UMETA(DisplayName = "Abandoned"),
    Occupied        UMETA(DisplayName = "Occupied"),
    UnderConstruction UMETA(DisplayName = "Under Construction")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Hut;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureState State = EArch_StructureState::Intact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float ConstructionProgress = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 Capacity = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 CurrentOccupants = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DurabilityPercent = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float AgeInDays = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasFireplace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsDefensive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString OwnerTribeName = TEXT("Unknown");

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Hut;
        State = EArch_StructureState::Intact;
        ConstructionProgress = 1.0f;
        Capacity = 4;
        CurrentOccupants = 0;
        DurabilityPercent = 100.0f;
        AgeInDays = 0.0f;
        bHasFireplace = false;
        bHasStorage = false;
        bIsDefensive = false;
        OwnerTribeName = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_SettlementLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 MaxStructures = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bHasPalisade = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bHasCentralFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FString SettlementName = TEXT("Unnamed Settlement");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 Population = 0;

    FArch_SettlementLayout()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 500.0f;
        MaxStructures = 20;
        bHasPalisade = false;
        bHasCentralFirePit = true;
        SettlementName = TEXT("Unnamed Settlement");
        Population = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Structure Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* CreateStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RemoveStructure(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureState(AActor* Structure, EArch_StructureState NewState);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AgeStructures(float DeltaTime);

    // Settlement Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateSettlement(FVector CenterLocation, float Radius, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateSettlementLayout(const FArch_SettlementLayout& Layout);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInRadius(FVector Center, float Radius);

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateInterior(AActor* Structure, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddInteriorDetails(AActor* Structure, bool bAddFurniture, bool bAddTools, bool bAddPersonalItems);

    // Procedural Generation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateRandomSettlement(FVector Location, int32 MinStructures, int32 MaxStructures);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector FindOptimalStructureLocation(FVector CenterPoint, float MinDistance, float MaxDistance);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EArch_StructureType GetRandomStructureType();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float CalculateStructureDurability(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void DebugCreateTestSettlement();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<AActor*> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_SettlementLayout CurrentSettlement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructureAgingRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoAgeStructures = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringFactor = 0.1f;

private:
    void InitializeStructureData(AActor* Structure, EArch_StructureType StructureType);
    void ApplyWeathering(AActor* Structure, float DeltaTime);
    FVector GetCircularPosition(FVector Center, float Radius, float Angle);
};