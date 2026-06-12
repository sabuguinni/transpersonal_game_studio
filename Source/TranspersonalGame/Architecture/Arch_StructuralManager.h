#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Arch_StructuralManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Foundation         UMETA(DisplayName = "Stone Foundation"),
    Wall              UMETA(DisplayName = "Stone Wall"),
    Archway           UMETA(DisplayName = "Stone Archway"),
    Circle            UMETA(DisplayName = "Stone Circle"),
    Shelter           UMETA(DisplayName = "Primitive Shelter"),
    Platform          UMETA(DisplayName = "Raised Platform")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Foundation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasVegetationGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 AgeInYears = 1000;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Foundation;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        bHasVegetationGrowth = true;
        AgeInYears = 1000;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructuralManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructuralManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructureSpawnRadius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoGenerateStructures = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnFoundationPattern(FVector CenterLocation, int32 Width = 3, int32 Height = 2);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStoneCircle(FVector CenterLocation, float Radius = 800.0f, int32 NumStones = 8);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(AActor* StructureActor, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddVegetationGrowth(AActor* StructureActor);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetNearbyStructures(FVector Location, float SearchRadius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CleanupOldStructures();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateTestStructures();

protected:
    UFUNCTION()
    void OnStructureInteraction(AActor* OverlappedActor, AActor* OtherActor);

    void CreateInteractionTrigger(AActor* StructureActor, FVector TriggerScale = FVector(5, 5, 3));

    AActor* SpawnStructureActor(EArch_StructureType StructureType, FVector Location, FRotator Rotation, FVector Scale);

    void ConfigureStructureMesh(AActor* StructureActor, EArch_StructureType StructureType);

private:
    TArray<AActor*> SpawnedStructures;
    TArray<ATriggerBox*> InteractionTriggers;
};