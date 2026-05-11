#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Quest_MassiveCrowdBattleSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_BattlePhase : uint8
{
    Preparation     UMETA(DisplayName = "Preparation"),
    Engagement      UMETA(DisplayName = "Engagement"),
    Climax          UMETA(DisplayName = "Climax"),
    Resolution      UMETA(DisplayName = "Resolution"),
    Aftermath       UMETA(DisplayName = "Aftermath")
};

UENUM(BlueprintType)
enum class EQuest_BattleObjective : uint8
{
    DefendSettlement    UMETA(DisplayName = "Defend Settlement"),
    HuntPredator        UMETA(DisplayName = "Hunt Predator"),
    SecureResources     UMETA(DisplayName = "Secure Resources"),
    RescueSurvivors     UMETA(DisplayName = "Rescue Survivors"),
    EliminateThreats    UMETA(DisplayName = "Eliminate Threats")
};

USTRUCT(BlueprintType)
struct FQuest_BattleConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    int32 MaxCrowdParticipants = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    float BattleDuration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    EQuest_BattleObjective PrimaryObjective = EQuest_BattleObjective::DefendSettlement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    TArray<FVector> StrategicPositions;

    FQuest_BattleConfiguration()
    {
        StrategicPositions.Add(FVector(1000, 1000, 100));
        StrategicPositions.Add(FVector(-1000, -1000, 100));
        StrategicPositions.Add(FVector(1000, -1000, 100));
        StrategicPositions.Add(FVector(-1000, 1000, 100));
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_MassiveCrowdBattleSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MassiveCrowdBattleSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* BattleZoneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    FQuest_BattleConfiguration BattleConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    EQuest_BattlePhase CurrentPhase = EQuest_BattlePhase::Preparation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    float BattleIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    int32 ActiveParticipants = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    TArray<AActor*> BattleParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    TArray<FVector> ObjectiveLocations;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void InitializeBattle(const FQuest_BattleConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void StartBattle();

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void EndBattle();

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void UpdateBattlePhase(EQuest_BattlePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void AddBattleParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void RemoveBattleParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void SetBattleObjective(EQuest_BattleObjective NewObjective);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    float CalculateBattleIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    bool IsBattleActive() const;

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    int32 GetActiveParticipantCount() const { return ActiveParticipants; }

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    EQuest_BattlePhase GetCurrentPhase() const { return CurrentPhase; }

protected:
    UFUNCTION()
    void OnBattleZoneEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                          bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnBattleZoneExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void UpdateBattleIntensity(float DeltaTime);
    void ManageCrowdBehavior();
    void ProcessBattleObjectives();
    void HandlePhaseTransition();

    float BattleTimer = 0.0f;
    bool bBattleActive = false;
    float LastIntensityUpdate = 0.0f;
};