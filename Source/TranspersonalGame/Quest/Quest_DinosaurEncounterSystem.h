#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Quest_DinosaurEncounterSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor         UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class EQuest_EncounterType : uint8
{
    Peaceful       UMETA(DisplayName = "Peaceful Observation"),
    Territorial    UMETA(DisplayName = "Territorial Warning"),
    Hunting        UMETA(DisplayName = "Hunting Behavior"),
    Feeding        UMETA(DisplayName = "Feeding Time"),
    Migration      UMETA(DisplayName = "Migration Movement")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DinosaurEncounter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    EQuest_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    EQuest_EncounterType EncounterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FString EncounterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsActive;

    FQuest_DinosaurEncounter()
    {
        Species = EQuest_DinosaurSpecies::Raptor;
        EncounterType = EQuest_EncounterType::Peaceful;
        Location = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        EncounterDescription = TEXT("A dinosaur encounter");
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_DinosaurEncounterSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_DinosaurEncounterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* DetectionSphere;

    // Encounter Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_DinosaurEncounter> ActiveEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float EncounterCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxSimultaneousEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bDebugMode;

    // Quest Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FString CurrentQuestObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bQuestEncounterActive;

    // Encounter Methods
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateDinosaurEncounter(EQuest_DinosaurSpecies Species, EQuest_EncounterType Type, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void TriggerEncounter(int32 EncounterIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteEncounter(int32 EncounterIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsPlayerInEncounterRange(const FQuest_DinosaurEncounter& Encounter);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestObjective(const FString& NewObjective);

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void PlayEncounterAudio(EQuest_DinosaurSpecies Species, EQuest_EncounterType Type);

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurEncounterTriggered, EQuest_DinosaurSpecies, Species, EQuest_EncounterType, Type);
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnDinosaurEncounterTriggered OnEncounterTriggered;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestObjectiveUpdated, const FString&, NewObjective);
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestObjectiveUpdated OnObjectiveUpdated;

private:
    float LastEncounterCheckTime;
    int32 ActiveEncounterCount;

    void CheckForPlayerEncounters();
    void SpawnDinosaurAtLocation(EQuest_DinosaurSpecies Species, FVector Location);
    FString GetSpeciesName(EQuest_DinosaurSpecies Species);
    FString GetEncounterTypeName(EQuest_EncounterType Type);
};