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
    CaveDwelling,
    WoodenShelter,
    StoneSite,
    CliffDwelling,
    DefensiveOutpost,
    StorageCache
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Stone,
    Wood,
    Bone,
    Hide,
    Vine,
    Mud
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<EArch_ConstructionMaterial> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DefenseRating;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::CaveDwelling;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        StructuralIntegrity = 100.0f;
        bIsHabitable = true;
        MaxOccupants = 1;
        DefenseRating = 0.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_StructureData> RegisteredStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float StructureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bAutoGenerateStructures;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveStructure(const FVector& Location, float Radius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateCaveDwelling(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateWoodenShelter(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStoneSite(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateCliffDwelling(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateStructurePlacement(const FVector& Location, EArch_StructureType StructureType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(const FVector& Location, float IntegrityChange);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateTestStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetStructureCount() const { return RegisteredStructures.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();
};