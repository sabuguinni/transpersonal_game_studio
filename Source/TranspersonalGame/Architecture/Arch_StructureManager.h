#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "Arch_StructureManager.generated.h"

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuin;

    FArch_StructureData()
    {
        StructureName = TEXT("Unknown");
        BiomeType = EBiomeType::Savanna;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        StructureHealth = 100.0f;
        bIsRuin = false;
    }
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    StoneWall       UMETA(DisplayName = "Stone Wall"),
    StoneArch       UMETA(DisplayName = "Stone Arch"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoGenerateStructures;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructuresForBiome(EBiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearStructuresInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureHealth(int32 StructureIndex, float NewHealth);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void RegenerateAllStructures();

protected:
    UFUNCTION()
    void OnStructureDestroyed(AActor* DestroyedActor);

    FVector GetBiomeCenterLocation(EBiomeType BiomeType);
    UStaticMesh* GetMeshForStructureType(EArch_StructureType StructureType);
    void ValidateStructurePlacement(FVector& Location, EBiomeType BiomeType);
};