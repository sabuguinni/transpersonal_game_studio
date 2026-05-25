#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

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
    float StructureHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable;

    FArch_StructureData()
    {
        StructureName = TEXT("DefaultStructure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        BiomeType = EBiomeType::Savana;
        StructureHealth = 100.0f;
        bIsHabitable = false;
    }
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Dwelling        UMETA(DisplayName = "Dwelling"),
    Storage         UMETA(DisplayName = "Storage"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Ceremonial      UMETA(DisplayName = "Ceremonial"),
    Workshop        UMETA(DisplayName = "Workshop"),
    Ruins           UMETA(DisplayName = "Ruins")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(const FArch_StructureData& InStructureData, EArch_StructureType InType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool CanPlayerEnter() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void DamageStructure(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    FArch_StructureData GetStructureData() const { return StructureData; }
};

UCLASS()
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY()
    TArray<AArch_StructureActor*> ManagedStructures;

    UPROPERTY()
    TMap<EBiomeType, TArray<FArch_StructureData>> BiomeStructures;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArch_StructureActor* SpawnStructure(const FArch_StructureData& StructureData, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArch_StructureActor*> GetStructuresInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(AArch_StructureActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterStructure(AArch_StructureActor* Structure);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateArchitecturalLayout();
};