#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../../SharedTypes.h"
#include "NPC_SocialMemorySystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TWeakObjectPtr<APawn> TargetPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float Relationship = 0.0f; // -100 to 100 (hostile to friendly)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    int32 InteractionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    FVector LastSeenLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    bool bIsHostile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    bool bIsAlly = false;

    FNPC_SocialMemory()
    {
        TargetPawn = nullptr;
        Relationship = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
        LastSeenLocation = FVector::ZeroVector;
        bIsHostile = false;
        bIsAlly = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialMemorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialMemorySystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TArray<FNPC_SocialMemory> SocialMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float MaxMemoryDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    int32 MaxMemoryEntries = 50;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void RecordInteraction(APawn* TargetPawn, float RelationshipChange, bool bWasHostile = false);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    FNPC_SocialMemory* GetMemoryOfPawn(APawn* TargetPawn);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    float GetRelationshipWith(APawn* TargetPawn);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    bool IsHostileTowards(APawn* TargetPawn);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    bool IsAllyWith(APawn* TargetPawn);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void UpdateMemoryLocation(APawn* TargetPawn, FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void DecayMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    TArray<APawn*> GetKnownHostiles();

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    TArray<APawn*> GetKnownAllies();

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void ForgetPawn(APawn* TargetPawn);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void ClearAllMemories();
};