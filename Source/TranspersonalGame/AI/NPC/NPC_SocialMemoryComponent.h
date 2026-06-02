#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_SocialMemoryComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    AActor* RememberedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float RelationshipValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    ENPCBehaviorState LastObservedBehavior;

    FNPC_SocialMemory()
    {
        RememberedActor = nullptr;
        RelationshipValue = 0.0f;
        LastInteractionTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        LastObservedBehavior = ENPCBehaviorState::Idle;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialMemoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialMemoryComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TArray<FNPC_SocialMemory> SocialMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float MaxMemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    int32 MaxMemoryEntries;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void RecordInteraction(AActor* OtherActor, float InteractionValue, ENPCBehaviorState ObservedBehavior);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    FNPC_SocialMemory* GetMemoryOf(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    float GetRelationshipValue(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    TArray<AActor*> GetKnownActors();

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void ForgetActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void DecayMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    AActor* GetClosestRememberedActor(const FVector& Location, float MaxDistance = 5000.0f);
};