#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Ruins,
    CaveDwelling,
    StonePillar,
    Archway,
    Platform,
    Shelter
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Ruins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WearLevel = 0.5f; // 0.0 = new, 1.0 = heavily weathered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasCarvedSymbols = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Ruins;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WearLevel = 0.5f;
        bHasMossGrowth = true;
        bHasCarvedSymbols = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PrimaryStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    TArray<UStaticMesh*> RuinsMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    TArray<UStaticMesh*> CaveDwellingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    TArray<UStaticMesh*> PillarMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    TArray<FVector> BiomeLocations;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeatheringEffect(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SpawnStructureAtBiome(int32 BiomeIndex, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void DistributeStructuresAcrossBiomes();

    UFUNCTION(BlueprintCallable, Category = "Structure Management", CallInEditor)
    void GenerateRandomStructures();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    FArch_StructureData GetStructureData() const { return StructureData; }

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureData(const FArch_StructureData& NewData);

protected:
    UFUNCTION()
    void UpdateStructureMesh();

    UFUNCTION()
    void ApplyMossEffect();

    UFUNCTION()
    void ApplyCarvedSymbols();

    UFUNCTION()
    UStaticMesh* GetMeshForStructureType(EArch_StructureType Type);
};