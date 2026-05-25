#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Arch_StructureManager.generated.h"

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
        StructureName = TEXT("DefaultStructure");
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
    Shelter     UMETA(DisplayName = "Shelter"),
    Ruin        UMETA(DisplayName = "Ruin"),
    Wall        UMETA(DisplayName = "Wall"),
    Platform    UMETA(DisplayName = "Platform"),
    Bridge      UMETA(DisplayName = "Bridge")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxStructureDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoSpawnStructures;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(const FVector& Location, EArch_StructureType StructureType, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveStructureAtLocation(const FVector& Location, float SearchRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(const FVector& Location, float IntegrityChange);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void PopulateBiomeWithStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType) const;

private:
    void InitializeStructureDatabase();
    void SpawnStructureActor(const FArch_StructureData& StructureData, EArch_StructureType StructureType);
    FString GetStructureMeshPath(EArch_StructureType StructureType, EBiomeType BiomeType) const;
};