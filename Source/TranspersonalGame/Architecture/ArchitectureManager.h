#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None = 0,
    PrimitiveHut,
    CaveDwelling,
    ElevatedShelter,
    TribalMeetingPlace,
    StorageCache,
    DefensiveWall,
    WatchTower,
    CraftingArea,
    FirePit
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone = 0,
    Wood,
    Thatch,
    AnimalHide,
    Bone,
    Clay,
    Vine,
    Mud
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<EArch_MaterialType> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DurabilityPercent = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 1;

    FArch_StructureData()
    {
        Materials.Add(EArch_MaterialType::Stone);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> PropNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> PropLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasSleepingArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasStorageArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float ComfortLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float WarmthLevel = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_StructureData> ActiveStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<EArch_StructureType, FArch_InteriorData> InteriorTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float StructureSpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    int32 MaxStructuresPerArea = 10;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool SpawnStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveStructure(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetNearbyStructures(FVector PlayerLocation, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanBuildAtLocation(FVector Location, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupInteriorForStructure(int32 StructureIndex, const FArch_InteriorData& InteriorData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_InteriorData GetInteriorTemplate(EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureDurability(int32 StructureIndex, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(int32 StructureIndex, float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetStructureCount() const { return ActiveStructures.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationInsideStructure(FVector TestLocation, int32 StructureIndex);

private:
    void InitializeInteriorTemplates();
    AActor* CreateStructureMesh(EArch_StructureType StructureType, FVector Location, FRotator Rotation, float Scale);
    void SetupStructureMaterials(AActor* StructureActor, const TArray<EArch_MaterialType>& Materials);
    bool ValidateStructurePlacement(FVector Location, EArch_StructureType StructureType);
};