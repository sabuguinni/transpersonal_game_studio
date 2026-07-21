#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Foundation     UMETA(DisplayName = "Foundation"),
    Wall          UMETA(DisplayName = "Wall"),
    Roof          UMETA(DisplayName = "Roof"),
    Door          UMETA(DisplayName = "Door"),
    Window        UMETA(DisplayName = "Window"),
    Pillar        UMETA(DisplayName = "Pillar"),
    Platform      UMETA(DisplayName = "Platform"),
    Stairs        UMETA(DisplayName = "Stairs")
};

UENUM(BlueprintType)
enum class EArch_ConstructionMaterial : uint8
{
    Stone         UMETA(DisplayName = "Stone"),
    Wood          UMETA(DisplayName = "Wood"),
    Clay          UMETA(DisplayName = "Clay"),
    Bone          UMETA(DisplayName = "Bone"),
    Hide          UMETA(DisplayName = "Hide"),
    Thatch        UMETA(DisplayName = "Thatch")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Foundation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_ConstructionMaterial Material = EArch_ConstructionMaterial::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherResistance = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 ConstructionCost = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsCompleted = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Foundation;
        Material = EArch_ConstructionMaterial::Stone;
        StructuralIntegrity = 100.0f;
        WeatherResistance = 80.0f;
        ConstructionCost = 10;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FVector> SnapPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float SnapRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxHealth = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float CurrentHealth = 200.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType Type, EArch_ConstructionMaterial Material);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanSnapToLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetNearestSnapPoint(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CompleteConstruction();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsStructureStable() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureDamaged(float DamageAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureDestroyed();

private:
    void UpdateStructuralIntegrity();
    void ApplyWeatherEffects(float DeltaTime);
    void UpdateMeshMaterial();
};