#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "Arch_ConstructionSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Clay            UMETA(DisplayName = "Clay"),
    Plant           UMETA(DisplayName = "Plant Fiber")
};

UENUM(BlueprintType)
enum class EArch_StructureIntegrity : uint8
{
    Unstable        UMETA(DisplayName = "Unstable"),
    Weak            UMETA(DisplayName = "Weak"),
    Stable          UMETA(DisplayName = "Stable"),
    Strong          UMETA(DisplayName = "Strong"),
    Reinforced      UMETA(DisplayName = "Reinforced")
};

USTRUCT(BlueprintType)
struct FArch_ConstructionMaterial
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    EArch_MaterialType MaterialType = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float Durability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float WeatherResistance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    float StructuralStrength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    int32 RequiredQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    bool bIsAvailable = false;
};

USTRUCT(BlueprintType)
struct FArch_ConstructionBlueprint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
    FString StructureName = TEXT("Basic Shelter");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
    TArray<FArch_ConstructionMaterial> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
    float ConstructionTime = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
    int32 RequiredBuilders = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
    EArch_StructureIntegrity TargetIntegrity = EArch_StructureIntegrity::Stable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
    FVector StructureSize = FVector(300.0f, 300.0f, 200.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ConstructionSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_ConstructionSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* ConstructionRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FoundationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UStaticMeshComponent*> StructureComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    FArch_ConstructionBlueprint CurrentBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    float ConstructionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    EArch_StructureIntegrity CurrentIntegrity = EArch_StructureIntegrity::Unstable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    TArray<AActor*> ActiveBuilders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    bool bIsUnderConstruction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    bool bSnapToGrid = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
    float GridSize = 100.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Construction")
    bool CanStartConstruction();

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void StartConstruction(const FArch_ConstructionBlueprint& Blueprint);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void AddBuilder(AActor* Builder);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void RemoveBuilder(AActor* Builder);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void AddMaterial(EArch_MaterialType MaterialType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    bool HasRequiredMaterials() const;

    UFUNCTION(BlueprintCallable, Category = "Construction")
    float GetConstructionProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Construction")
    EArch_StructureIntegrity GetStructureIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void CompleteConstruction();

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void CancelConstruction();

    UFUNCTION(BlueprintCallable, Category = "Construction")
    FVector SnapToGrid(const FVector& Location) const;

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Construction")
    void OnConstructionStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Construction")
    void OnConstructionProgress(float Progress);

    UFUNCTION(BlueprintImplementableEvent, Category = "Construction")
    void OnConstructionCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Construction")
    void OnConstructionCancelled();

private:
    void UpdateConstructionProgress(float DeltaTime);
    void UpdateStructureIntegrity();
    void UpdateVisualComponents();
    float CalculateConstructionSpeed() const;
    bool ValidateMaterials() const;
};