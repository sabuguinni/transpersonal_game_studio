#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCInteractionComponent.generated.h"

class ANPCBehaviorController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCInteraction, AActor*, InteractingActor, ENPC_InteractionType, InteractionType);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCInteractionComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Interaction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<ENPC_InteractionType> AvailableInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanInteractWithPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresLineOfSight;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ENPC_SocialStatus SocialStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float AggressionLevel;

    // Relationship System
    UPROPERTY(BlueprintReadOnly, Category = "Relationships")
    TMap<AActor*, FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    float RelationshipDecayRate;

    // Dialogue System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> GreetingLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> FearLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> AggressiveLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> IdleLines;

public:
    // Interaction Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNPCInteraction OnNPCInteraction;

    // Interaction Functions
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanInteractWith(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool TryInteract(AActor* InteractingActor, ENPC_InteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    TArray<ENPC_InteractionType> GetAvailableInteractions(AActor* InteractingActor) const;

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyTrust(float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyFear(float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyAggression(float AggressionChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    ENPC_SocialStatus GetSocialStatus() const { return SocialStatus; }

    // Relationship Functions
    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void UpdateRelationship(AActor* OtherActor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    float GetRelationshipValue(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    ENPC_RelationshipType GetRelationshipType(AActor* OtherActor) const;

    // Dialogue Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetDialogueLine(ENPC_BehaviorState BehaviorState) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SpeakLine(const FString& Line);

protected:
    // Internal Functions
    void UpdateSocialStatus();
    void UpdateRelationships(float DeltaTime);
    bool HasLineOfSight(AActor* TargetActor) const;
    void ProcessInteraction(AActor* InteractingActor, ENPC_InteractionType InteractionType);

private:
    // Internal State
    TArray<AActor*> NearbyActors;
    float LastInteractionTime;
    AActor* LastInteractingActor;

    // Helper Functions
    void UpdateNearbyActors();
    float CalculateRelationshipModifier(AActor* OtherActor) const;
};