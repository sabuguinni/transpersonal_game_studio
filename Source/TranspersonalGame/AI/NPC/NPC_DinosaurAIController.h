#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "NPC_DinosaurAIController.generated.h"

UCLASS()
class TRANSPERSONALGAME_API ANPC_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

public:
    // Behavior Tree Asset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // Blackboard Asset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComp;

    // Sight Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float MaxAge;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValueAsVector(const FName& KeyName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValueAsBool(const FName& KeyName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValueAsFloat(const FName& KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValueAsObject(const FName& KeyName, UObject* Value);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    void ConfigurePerception();
    void SetupBlackboardDefaults();

    // Cached references
    class UNPC_DinosaurBehaviorManager* BehaviorManager;
    APawn* PlayerPawn;
};