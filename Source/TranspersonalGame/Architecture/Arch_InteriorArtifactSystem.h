#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Arch_InteriorArtifactSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ArtifactType : uint8
{
    None = 0,
    StoneTools,
    BoneFragments,
    PrimitivePottery,
    AnimalHides,
    FirePit,
    FoodRemains,
    WaterContainer,
    SleepingArea,
    WeaponCache,
    RitualObjects
};

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    None = 0,
    Cave,
    RockShelter,
    LeanTo,
    PitHouse,
    TreeShelter,
    CliffDwelling,
    TemporaryHut,
    UndergroundBurrow,
    StoneChamber,
    NaturalAlcove
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ArtifactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    EArch_ArtifactType ArtifactType = EArch_ArtifactType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FString ArtifactName = TEXT("Unknown Artifact");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FString Description = TEXT("An ancient artifact left by prehistoric inhabitants");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    float AgeInYears = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    float ConditionPercent = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FVector RelativePosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    bool bIsInteractable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FString InteractionText = TEXT("Examine artifact");

    FArch_ArtifactData()
    {
        ArtifactType = EArch_ArtifactType::None;
        ArtifactName = TEXT("Unknown Artifact");
        Description = TEXT("An ancient artifact left by prehistoric inhabitants");
        AgeInYears = 10000.0f;
        ConditionPercent = 50.0f;
        RelativePosition = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        bIsInteractable = true;
        InteractionText = TEXT("Examine artifact");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_DwellingType DwellingType = EArch_DwellingType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FArch_ArtifactData> Artifacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector InteriorBounds = FVector(500.0f, 500.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float HabitationDuration = 365.0f; // Days

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasSleepingArea = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasToolStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString InhabitantStory = TEXT("A family of early humans lived here during the harsh winter months");

    FArch_InteriorLayout()
    {
        DwellingType = EArch_DwellingType::Cave;
        InteriorBounds = FVector(500.0f, 500.0f, 300.0f);
        MaxOccupants = 2;
        HabitationDuration = 365.0f;
        bHasFirePit = true;
        bHasSleepingArea = true;
        bHasToolStorage = true;
        InhabitantStory = TEXT("A family of early humans lived here during the harsh winter months");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorArtifact : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorArtifact();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ArtifactMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact Data")
    FArch_ArtifactData ArtifactData;

public:
    UFUNCTION(BlueprintCallable, Category = "Artifact")
    void SetArtifactData(const FArch_ArtifactData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Artifact")
    FArch_ArtifactData GetArtifactData() const { return ArtifactData; }

    UFUNCTION(BlueprintCallable, Category = "Artifact")
    void OnPlayerInteract();

    UFUNCTION(BlueprintImplementableEvent, Category = "Artifact")
    void OnArtifactExamined();

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrehistoricDwelling : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricDwelling();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* InteriorBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    FArch_InteriorLayout InteriorLayout;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Artifacts")
    TArray<AArch_InteriorArtifact*> SpawnedArtifacts;

public:
    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void GenerateInteriorArtifacts();

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void ClearInteriorArtifacts();

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void SetInteriorLayout(const FArch_InteriorLayout& NewLayout);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    FArch_InteriorLayout GetInteriorLayout() const { return InteriorLayout; }

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    TArray<AArch_InteriorArtifact*> GetArtifacts() const { return SpawnedArtifacts; }

protected:
    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    FVector GetRandomPositionInBounds() const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void SpawnArtifact(const FArch_ArtifactData& ArtifactData);
};

UCLASS()
class TRANSPERSONALGAME_API UArch_InteriorArtifactSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void RegisterDwelling(AArch_PrehistoricDwelling* Dwelling);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void UnregisterDwelling(AArch_PrehistoricDwelling* Dwelling);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    TArray<AArch_PrehistoricDwelling*> GetAllDwellings() const { return RegisteredDwellings; }

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void GenerateRandomInteriorStory(FArch_InteriorLayout& Layout);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    TArray<FArch_ArtifactData> GenerateArtifactsForDwelling(EArch_DwellingType DwellingType, int32 MaxOccupants);

protected:
    UPROPERTY()
    TArray<AArch_PrehistoricDwelling*> RegisteredDwellings;

    UPROPERTY()
    TArray<FString> InteriorStoryTemplates;

    void InitializeStoryTemplates();
    FArch_ArtifactData CreateRandomArtifact(EArch_ArtifactType Type);
};