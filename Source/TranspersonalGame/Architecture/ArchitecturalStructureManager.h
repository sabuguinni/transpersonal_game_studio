#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin"),
    NaturalArch     UMETA(DisplayName = "Natural Arch")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsInteractable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StonePillar;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeName = TEXT("Unknown");
        bIsInteractable = false;
        StructuralIntegrity = 100.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> StructureRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxSpawnDistance = 100000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoSpawnStructures = true;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation, FString BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearStructuresInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void SpawnStructuresForAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetTotalStructureCount() const;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedStructureActors;

    void InitializeBiomeStructures();
    AActor* CreateStructureActor(EArch_StructureType StructureType, FVector Location, FRotator Rotation);
    FString GetStructureMeshPath(EArch_StructureType StructureType) const;
};