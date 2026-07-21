#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Narr_NPCCharacter.generated.h"

UENUM(BlueprintType)
enum class ENarr_NPCState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Talking     UMETA(DisplayName = "Talking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following")
};

UENUM(BlueprintType)
enum class ENarr_NPCType : uint8
{
    Survivor    UMETA(DisplayName = "Survivor"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Guide       UMETA(DisplayName = "Guide"),
    Trader      UMETA(DisplayName = "Trader")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENarr_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> DialogueOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bCanTrade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bCanGiveQuests;

    FNarr_NPCData()
    {
        NPCName = TEXT("Unknown");
        NPCType = ENarr_NPCType::Survivor;
        InteractionRange = 200.0f;
        bCanTrade = false;
        bCanGiveQuests = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_NPCCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANarr_NPCCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void StartDialogue(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void SetNPCState(ENarr_NPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    bool CanInteract() const;

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void FleeFromDanger(FVector DangerLocation);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FNarr_NPCData NPCData;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    ENarr_NPCState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    AActor* CurrentInteractingPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float MovementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    FVector CurrentPatrolTarget;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    float StateTimer;

private:
    void UpdatePatrolling(float DeltaTime);
    void UpdateFleeing(float DeltaTime);
    void GenerateNewPatrolTarget();
    
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                        bool bFromSweep, const FHitResult& SweepResult);
    
    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};