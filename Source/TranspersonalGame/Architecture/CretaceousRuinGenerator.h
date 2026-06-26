#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "CretaceousRuinGenerator.generated.h"

/**
 * Enum for ruin structure types found in Cretaceous-era prehistoric environments.
 * Prefix: Arch_ to avoid conflicts with other agents.
 */
UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    StoneColumn     UMETA(DisplayName = "Stone Column"),
    WallSegment     UMETA(DisplayName = "Wall Segment"),
    ArchGateway     UMETA(DisplayName = "Arch Gateway"),
    AltarStone      UMETA(DisplayName = "Altar Stone"),
    RubblePile      UMETA(DisplayName = "Rubble Pile"),
    FoundationSlab  UMETA(DisplayName = "Foundation Slab")
};

/**
 * Data struct describing a single ruin element placement.
 */
USTRUCT(BlueprintType)
struct FArch_RuinElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    EArch_RuinType RuinType = EArch_RuinType::StoneColumn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    FVector RelativeOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    float YawRotation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    float WearLevel = 0.5f; // 0=pristine, 1=heavily damaged
};

/**
 * ACretaceousRuinGenerator
 *
 * Procedurally generates Cretaceous-era stone ruin complexes.
 * Spawns columns, walls, rubble, and altar stones to create
 * historically-grounded prehistoric structures.
 *
 * Agent #07 — Architecture & Interior Agent
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Ruin Generator"))
class TRANSPERSONALGAME_API ACretaceousRuinGenerator : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousRuinGenerator();

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Layout")
    int32 NumColumns = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Layout")
    float RuinRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Layout")
    bool bSpawnArchGateway = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Layout")
    bool bSpawnAltarStone = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Layout")
    int32 NumRubblePieces = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Layout")
    int32 RandomSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Wear")
    float GlobalWearLevel = 0.7f; // Cretaceous ruins are heavily worn

    // --- Mesh References ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Meshes")
    TObjectPtr<UStaticMesh> ColumnMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Meshes")
    TObjectPtr<UStaticMesh> WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Meshes")
    TObjectPtr<UStaticMesh> RubbleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin|Meshes")
    TObjectPtr<UStaticMesh> AltarMesh;

    // --- Runtime State ---

    UPROPERTY(BlueprintReadOnly, Category = "Ruin|State")
    TArray<FArch_RuinElement> SpawnedElements;

    UPROPERTY(BlueprintReadOnly, Category = "Ruin|State")
    bool bRuinGenerated = false;

    // --- Methods ---

    UFUNCTION(BlueprintCallable, Category = "Ruin|Generation")
    void GenerateRuin();

    UFUNCTION(BlueprintCallable, Category = "Ruin|Generation")
    void ClearRuin();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Ruin|Generation")
    void RegenerateInEditor();

    UFUNCTION(BlueprintCallable, Category = "Ruin|Query")
    int32 GetElementCount() const;

    UFUNCTION(BlueprintCallable, Category = "Ruin|Query")
    FVector GetRuinCenter() const;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> RuinComponents;

    void SpawnColumns();
    void SpawnWalls();
    void SpawnArchGateway();
    void SpawnAltarStone();
    void SpawnRubble();
    UStaticMeshComponent* AddRuinMeshComponent(UStaticMesh* Mesh, FVector Offset, FRotator Rotation, FVector Scale);
};
