#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "../Core/SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    ElevatedShelter UMETA(DisplayName = "Elevated Shelter"),
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    WoodenPlatform  UMETA(DisplayName = "Wooden Platform"),
    RockFormation   UMETA(DisplayName = "Rock Formation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsInteractable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 4;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::CaveDwelling;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savanna;
        bIsInteractable = true;
        StructuralIntegrity = 100.0f;
        MaxOccupants = 4;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ArchitecturalStructure : public AActor
{
    GENERATED_BODY()

public:
    AArch_ArchitecturalStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Mesh")
    UStaticMesh* DefaultMesh;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeStructure(const FArch_StructureData& InStructureData);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    EArch_StructureType GetStructureType() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructuralIntegrity(float NewIntegrity);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool CanAccommodateOccupants(int32 RequestedOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void UpdateMeshForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnStructureInteracted(AActor* InteractingActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnStructureDamaged(float DamageAmount);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_ArchitecturalStructureManager : public UObject
{
    GENERATED_BODY()

public:
    UArch_ArchitecturalStructureManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    static UArch_ArchitecturalStructureManager* GetInstance(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    AArch_ArchitecturalStructure* SpawnStructure(UWorld* World, const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    void SpawnStructuresForBiome(UWorld* World, EBiomeType BiomeType, int32 StructureCount = 10);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    TArray<AArch_ArchitecturalStructure*> GetStructuresInRadius(UWorld* World, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    void UpdateAllStructures(UWorld* World, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    int32 GetTotalStructureCount(UWorld* World) const;

protected:
    UPROPERTY()
    TArray<TWeakObjectPtr<AArch_ArchitecturalStructure>> ManagedStructures;

    FArch_StructureData GenerateStructureDataForBiome(EBiomeType BiomeType, FVector BaseLocation);
    FVector GetBiomeBaseLocation(EBiomeType BiomeType);
    EArch_StructureType GetPreferredStructureTypeForBiome(EBiomeType BiomeType);
};