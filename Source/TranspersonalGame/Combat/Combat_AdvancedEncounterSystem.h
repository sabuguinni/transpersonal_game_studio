#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Combat_AdvancedEncounterSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_EncounterType : uint8
{
    ApexPredator    UMETA(DisplayName = "Apex Predator"),
    PackHunting     UMETA(DisplayName = "Pack Hunting"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Ambush          UMETA(DisplayName = "Ambush"),
    Patrol          UMETA(DisplayName = "Patrol")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme"),
    Lethal      UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_EncounterType EncounterType = ECombat_EncounterType::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 DinosaurCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float ActivationRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float EngagementDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FString EncounterName = TEXT("Generic Encounter");

    FCombat_EncounterData()
    {
        EncounterType = ECombat_EncounterType::Patrol;
        ThreatLevel = ECombat_ThreatLevel::Medium;
        DinosaurCount = 1;
        ActivationRange = 1000.0f;
        EngagementDuration = 60.0f;
        bRequiresLineOfSight = true;
        EncounterName = TEXT("Generic Encounter");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString FormationName = TEXT("Standard");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bMaintainFormation = true;

    FCombat_TacticalFormation()
    {
        FormationName = TEXT("Standard");
        FormationRadius = 500.0f;
        bMaintainFormation = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AdvancedEncounterSystem : public AActor
{
    GENERATED_BODY()

public:
    ACombat_AdvancedEncounterSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UTriggerBox* EncounterTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Configuration")
    FCombat_EncounterData EncounterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_TacticalFormation TacticalFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Configuration")
    TArray<TSubclassOf<class APawn>> DinosaurClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<class APawn*> SpawnedDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    class APawn* PlayerPawn;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bEncounterActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float EncounterTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* EncounterStartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* EncounterEndSound;

public:
    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void StartEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void EndEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void SpawnDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void DespawnDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalFormation();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AssignDinosaurRoles();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnEncounterStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnEncounterEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnPlayerDetected(APawn* DetectedPlayer);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, 
                              class UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, 
                            class UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void UpdateEncounterLogic(float DeltaTime);
    void CheckPlayerDistance();
    void UpdateDinosaurBehavior();
    FVector GetFormationPosition(int32 DinosaurIndex) const;
    bool IsPlayerInLineOfSight() const;
};