#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorTypes.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBehaviorStateChanged, EBehaviorState, OldState, EBehaviorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDomesticationChanged, EDomesticationLevel, OldLevel, EDomesticationLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryAdded, const FNPCMemory&, NewMemory);

/**
 * Componente central que gerencia o comportamento, memória e relações sociais de NPCs
 * Integra-se com Behavior Trees, AI Perception e sistemas de rotina diária
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ========== CONFIGURAÇÃO BASE ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FDataTableRowHandle SpeciesData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EBehaviorState CurrentBehaviorState = EBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    EDomesticationLevel DomesticationLevel = EDomesticationLevel::Wild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationProgress = 0.0f; // 0.0 - 1.0

    // ========== SISTEMA DE MEMÓRIA ==========
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPCMemory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f; // Taxa de esquecimento por hora

    // ========== RELAÇÕES SOCIAIS ==========
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TMap<TWeakObjectPtr<AActor>, FSocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialUpdateInterval = 5.0f; // Segundos entre atualizações sociais

    // ========== ROTINAS DIÁRIAS ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FDailyRoutine> DailyRoutines;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine")
    FDailyRoutine CurrentRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float RoutineFlexibility = 0.3f; // Quão facilmente abandona rotina por estímulos

    // ========== NECESSIDADES BÁSICAS ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Hunger = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Thirst = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Energy = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialNeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Safety = 1.0f;

    // ========== EVENTOS ==========
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBehaviorStateChanged OnBehaviorStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDomesticationChanged OnDomesticationChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMemoryAdded OnMemoryAdded;

    // ========== FUNÇÕES PÚBLICAS ==========
    
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(const FNPCMemory& NewMemory);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPCMemory> GetMemoriesOfType(FGameplayTag EventType, float MaxAge = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasMemoryOfActor(AActor* Actor, FGameplayTag EventType = FGameplayTag());

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialRelationship(AActor* OtherActor, float AffinityChange, float FamiliarityChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FSocialRelationship GetSocialRelationship(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessDomesticationInteraction(AActor* Player, float InteractionValue);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void UpdateDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FDailyRoutine GetCurrentRoutineForTime(EDayPeriod Period);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    EBehaviorState GetMostUrgentNeed();

    UFUNCTION(BlueprintPure, Category = "Species")
    FDinosaurSpeciesData GetSpeciesData();

    UFUNCTION(BlueprintPure, Category = "Behavior")
    bool CanBeDomesticated();

    UFUNCTION(BlueprintPure, Category = "Behavior")
    float GetAggressionLevel();

    UFUNCTION(BlueprintPure, Category = "Behavior")
    float GetSocialTendency();

protected:
    // ========== FUNÇÕES INTERNAS ==========
    
    void InitializeFromSpeciesData();
    void CleanOldMemories();
    void UpdateSocialRelationships();
    void ProcessNaturalBehaviorChanges();
    
    EDayPeriod GetCurrentDayPeriod();
    float CalculateMemoryImportance(const FNPCMemory& Memory);
    void DecayMemories(float DeltaTime);

    // ========== TIMERS ==========
    
    UPROPERTY()
    float LastSocialUpdate = 0.0f;

    UPROPERTY()
    float LastRoutineUpdate = 0.0f;

    UPROPERTY()
    float LastMemoryCleanup = 0.0f;

    // ========== CACHE ==========
    
    UPROPERTY()
    FDinosaurSpeciesData CachedSpeciesData;

    UPROPERTY()
    bool bSpeciesDataCached = false;
};