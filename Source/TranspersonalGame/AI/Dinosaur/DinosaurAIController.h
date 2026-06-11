#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "SharedTypes.h"
#include "DinosaurAIController.generated.h"

UCLASS()
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

public:
    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Blackboard Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // AI Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    EDinosaurSpecies DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float TerritoryRadius;

    // Blackboard Keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard Keys")
    FName PlayerActorKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard Keys")
    FName PatrolLocationKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard Keys")
    FName HomeLocationKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard Keys")
    FName CurrentStateKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard Keys")
    FName TargetLocationKey;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValueAsVector(const FName& KeyName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValueAsObject(const FName& KeyName, UObject* Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValueAsEnum(const FName& KeyName, uint8 Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    FVector GetBlackboardValueAsVector(const FName& KeyName);

    UFUNCTION(BlueprintCallable, Category = "AI")
    UObject* GetBlackboardValueAsObject(const FName& KeyName);

    UFUNCTION(BlueprintCallable, Category = "AI")
    uint8 GetBlackboardValueAsEnum(const FName& KeyName);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // AI Behavior Functions
    void SetupPerception();
    void SetupBlackboard();
    void InitializeDinosaurBehavior();
    
    // Species-specific behavior setup
    void SetupTRexBehavior();
    void SetupRaptorBehavior();
    void SetupTriceratopsBehavior();
    void SetupBrachiosaurusBehavior();
    void SetupPteranodonBehavior();

private:
    // Internal state
    bool bBehaviorTreeStarted;
    FVector HomeLocation;
    TArray<FVector> PatrolPoints;
    int32 CurrentPatrolIndex;
};

#include "DinosaurAIController.generated.h"