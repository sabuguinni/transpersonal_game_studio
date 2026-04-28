#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    StoneCircular UMETA(DisplayName = "Stone Circular Hut"),
    CaveInterior UMETA(DisplayName = "Cave Dwelling"),
    ElevatedPlatform UMETA(DisplayName = "Elevated Platform"),
    CommunalLonghouse UMETA(DisplayName = "Communal Longhouse"),
    TemporaryLeanTo UMETA(DisplayName = "Temporary Lean-To"),
    UndergroundBurrow UMETA(DisplayName = "Underground Burrow")
};

UENUM(BlueprintType)
enum class EArch_InteriorPropType : uint8
{
    FirePit UMETA(DisplayName = "Fire Pit"),
    SleepingArea UMETA(DisplayName = "Sleeping Area"),
    StorageBasket UMETA(DisplayName = "Storage Basket"),
    ToolRack UMETA(DisplayName = "Tool Rack"),
    CookingArea UMETA(DisplayName = "Cooking Area"),
    WorkBench UMETA(DisplayName = "Work Bench"),
    WaterContainer UMETA(DisplayName = "Water Container"),
    FoodStorage UMETA(DisplayName = "Food Storage")
};

USTRUCT(BlueprintType)
struct FArch_DwellingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingType DwellingType = EArch_DwellingType::StoneCircular;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FVector Size = FVector(400.0f, 400.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float StructuralIntegrity = 100.0f;

    FArch_DwellingConfig()
    {
        DwellingType = EArch_DwellingType::StoneCircular;
        Size = FVector(400.0f, 400.0f, 300.0f);
        MaxOccupants = 4;
        bHasFirePit = true;
        bHasStorage = true;
        StructuralIntegrity = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct FArch_InteriorProp
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorPropType PropType = EArch_InteriorPropType::FirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsActive = true;

    FArch_InteriorProp()
    {
        PropType = EArch_InteriorPropType::FirePit;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_DwellingConfig DwellingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_InteriorProp> InteriorProps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<UStaticMeshComponent*> StructuralComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<UStaticMeshComponent*> InteriorComponents;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void BuildDwelling(EArch_DwellingType DwellingType, FVector Size);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddInteriorProp(EArch_InteriorPropType PropType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveInteriorProp(int32 PropIndex);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void DamageStructure(float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetStructuralIntegrity() const { return DwellingConfig.StructuralIntegrity; }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintPure, Category = "Architecture")
    FVector GetDwellingSize() const { return DwellingConfig.Size; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetDwellingConfig(const FArch_DwellingConfig& NewConfig);

protected:
    UFUNCTION()
    void CreateStructuralComponents();

    UFUNCTION()
    void CreateInteriorComponents();

    UFUNCTION()
    void UpdateStructuralIntegrity();

    UFUNCTION()
    UStaticMeshComponent* CreateMeshComponent(const FString& ComponentName);

    UFUNCTION()
    void SetupStoneCircularHut();

    UFUNCTION()
    void SetupElevatedPlatform();

    UFUNCTION()
    void SetupCommunalLonghouse();

    UFUNCTION()
    void SetupCaveInterior();

private:
    float LastIntegrityCheck;
    bool bNeedsStructuralUpdate;
};