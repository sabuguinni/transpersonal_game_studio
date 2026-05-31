#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "ArchitecturalManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuin;

    FArch_StructureData()
    {
        StructureName = TEXT("Unknown");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        BiomeType = EBiomeType::Savanna;
        StructuralIntegrity = 100.0f;
        bIsRuin = false;
    }
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None = 0,
    Pillar = 1,
    Wall = 2,
    Temple = 3,
    Dwelling = 4,
    Monument = 5,
    Bridge = 6,
    Cave = 7,
    Ruin = 8
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitecturalManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitecturalManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Structure Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructuresAcrossBiomes(EArch_StructureType StructureType, int32 CountPerBiome);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetAllStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(AActor* StructureActor, float NewIntegrity);

    // Ruin System
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ConvertToRuin(AActor* StructureActor);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnRuinsInBiome(EBiomeType BiomeType, int32 RuinCount);

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupInteriorProps(AActor* StructureActor, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddPrimitiveArtifacts(AActor* StructureActor);

    // Biome Distribution
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void DistributeStructuresEvenly();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EBiomeType, FVector> BiomeCenters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoSpawnStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures;

private:
    void InitializeBiomeCenters();
    bool IsLocationValid(FVector Location, EBiomeType BiomeType);
    void RegisterStructure(AActor* StructureActor, EArch_StructureType StructureType, EBiomeType BiomeType);
    void CleanupExcessStructures();
};