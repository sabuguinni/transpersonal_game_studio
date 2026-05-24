#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurThreatLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Cautious    UMETA(DisplayName = "Cautious"), 
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurActivity : uint8
{
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Fighting    UMETA(DisplayName = "Fighting"),
    Socializing UMETA(DisplayName = "Socializing")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurThreatLevel ThreatLevel = ENPC_DinosaurThreatLevel::Cautious;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurActivity CurrentActivity = ENPC_DinosaurActivity::Patrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialLevel = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsTerritorial = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritoryRadius = 5000.0f;

    FNPC_DinosaurBehaviorProfile()
    {
        ThreatLevel = ENPC_DinosaurThreatLevel::Cautious;
        CurrentActivity = ENPC_DinosaurActivity::Patrolling;
        DetectionRadius = 2000.0f;
        AttackRadius = 500.0f;
        FleeRadius = 1000.0f;
        AggressionLevel = 0.5f;
        FearLevel = 0.3f;
        SocialLevel = 0.4f;
        bIsPackHunter = false;
        bIsTerritorial = false;
        bIsNocturnal = false;
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownPrey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> DangerousLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* LastAttacker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastPlayerSighting = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastAttack = 0.0f;

    FNPC_DinosaurMemory()
    {
        LastAttacker = nullptr;
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSinceLastPlayerSighting = 0.0f;
        TimeSinceLastAttack = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Tick function
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { return TStatId(); }

    // Gestão de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void RegisterDinosaur(APawn* Dinosaur, const FNPC_DinosaurBehaviorProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UnregisterDinosaur(APawn* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateDinosaurBehavior(APawn* Dinosaur, float DeltaTime);

    // Sistema de detecção
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ProcessPlayerDetection(APawn* Dinosaur, APawn* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ProcessThreatDetection(APawn* Dinosaur, AActor* Threat);

    // Sistema de memória
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateDinosaurMemory(APawn* Dinosaur, AActor* Actor, bool bIsThreat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FNPC_DinosaurMemory GetDinosaurMemory(APawn* Dinosaur);

    // Sistema de comportamento em grupo
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ProcessPackBehavior(APawn* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    TArray<APawn*> GetNearbyPackMembers(APawn* Dinosaur, float Radius = 3000.0f);

    // Configurações por espécie
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FNPC_DinosaurBehaviorProfile GetTRexProfile();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FNPC_DinosaurBehaviorProfile GetRaptorProfile();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FNPC_DinosaurBehaviorProfile GetHerbivoreProfile();

    // Utilitários
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerInRange(APawn* Dinosaur, float Range);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetRandomPatrolPoint(APawn* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurActivity(APawn* Dinosaur, ENPC_DinosaurActivity NewActivity);

protected:
    // Dados dos dinossauros registrados
    UPROPERTY()
    TMap<APawn*, FNPC_DinosaurBehaviorProfile> RegisteredDinosaurs;

    UPROPERTY()
    TMap<APawn*, FNPC_DinosaurMemory> DinosaurMemories;

    // Cache do jogador
    UPROPERTY()
    APawn* CachedPlayer;

    // Timers
    float PlayerSearchTimer = 0.0f;
    float BehaviorUpdateTimer = 0.0f;

    // Configurações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float PlayerSearchInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float BehaviorUpdateInterval = 0.5f;

private:
    void FindPlayer();
    void UpdateAllDinosaurs(float DeltaTime);
    void ProcessDinosaurAI(APawn* Dinosaur, const FNPC_DinosaurBehaviorProfile& Profile, float DeltaTime);
    void UpdateBlackboardValues(APawn* Dinosaur, const FNPC_DinosaurBehaviorProfile& Profile);
    float CalculateDistanceToPlayer(APawn* Dinosaur);
    bool ShouldAttackPlayer(APawn* Dinosaur, const FNPC_DinosaurBehaviorProfile& Profile, float PlayerDistance);
    bool ShouldFleeFromPlayer(APawn* Dinosaur, const FNPC_DinosaurBehaviorProfile& Profile, float PlayerDistance);
};