#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros Pequenos (Domesticáveis)
    Compsognathus     UMETA(DisplayName = "Compsognathus"),
    Parasaurolophus   UMETA(DisplayName = "Parasaurolophus"),
    Triceratops       UMETA(DisplayName = "Triceratops"),
    
    // Herbívoros Grandes (Neutros)
    Brachiosaurus     UMETA(DisplayName = "Brachiosaurus"),
    Diplodocus        UMETA(DisplayName = "Diplodocus"),
    Stegosaurus       UMETA(DisplayName = "Stegosaurus"),
    
    // Carnívoros Pequenos (Agressivos)
    Velociraptor      UMETA(DisplayName = "Velociraptor"),
    Deinonychus       UMETA(DisplayName = "Deinonychus"),
    
    // Carnívoros Grandes (Apex Predators)
    TyrannosaurusRex  UMETA(DisplayName = "Tyrannosaurus Rex"),
    Allosaurus        UMETA(DisplayName = "Allosaurus"),
    Spinosaurus       UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle              UMETA(DisplayName = "Idle"),
    Foraging          UMETA(DisplayName = "Foraging"),
    Hunting           UMETA(DisplayName = "Hunting"),
    Drinking          UMETA(DisplayName = "Drinking"),
    Resting           UMETA(DisplayName = "Resting"),
    Socializing       UMETA(DisplayName = "Socializing"),
    Territorial       UMETA(DisplayName = "Territorial"),
    Fleeing           UMETA(DisplayName = "Fleeing"),
    Investigating     UMETA(DisplayName = "Investigating"),
    Patrolling        UMETA(DisplayName = "Patrolling"),
    Mating            UMETA(DisplayName = "Mating"),
    Nesting           UMETA(DisplayName = "Nesting")
};

UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild              UMETA(DisplayName = "Wild"),
    Wary              UMETA(DisplayName = "Wary"),
    Curious           UMETA(DisplayName = "Curious"),
    Tolerant          UMETA(DisplayName = "Tolerant"),
    Friendly          UMETA(DisplayName = "Friendly"),
    Bonded            UMETA(DisplayName = "Bonded"),
    Domesticated      UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    // Traços de personalidade base (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerritorialInstinct = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;
    
    // Preferências comportamentais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredActivityTime = 0.5f; // 0.0 = nocturno, 0.5 = crepuscular, 1.0 = diurno
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialGroupPreference = 0.5f; // 0.0 = solitário, 1.0 = altamente social
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    // Memória de localizações importantes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownWaterSources;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownFoodSources;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SafeRestingSpots;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DangerousAreas;
    
    // Memória de outros actores
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, float> KnownActorsThreatLevel; // -1.0 (amigo) a 1.0 (inimigo)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, FDateTime> LastSeenActors;
    
    // Memória do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastPlayerEncounter;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerTrustLevel = 0.0f; // -1.0 (hostil) a 1.0 (confiança total)
    
    // Contador de interações positivas/negativas com o jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositivePlayerInteractions = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativePlayerInteractions = 0;
};

USTRUCT(BlueprintType)
struct FDinosaurNeeds
{
    GENERATED_BODY()

    // Necessidades básicas (0.0 = satisfeito, 1.0 = crítico)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Hunger = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Thirst = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fatigue = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialNeed = 0.4f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerritorialSecurity = 0.6f;
    
    // Necessidades reprodutivas (sazonais)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MatingUrge = 0.0f;
    
    // Taxa de degradação das necessidades (por hora)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerDecayRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThirstDecayRate = 0.15f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FatigueDecayRate = 0.05f;
};

/**
 * Componente principal que gere o comportamento complexo dos dinossauros NPCs
 * Integra Behavior Trees, memória persistente, personalidade e necessidades
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
    // === CONFIGURAÇÃO BÁSICA ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FDinosaurPersonality Personality;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FString IndividualName; // Nome único gerado proceduralmente
    
    // === BEHAVIOR TREE INTEGRATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardAsset* BlackboardAsset;
    
    // === ESTADO COMPORTAMENTAL ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    EDinosaurBehaviorState CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    FDinosaurNeeds CurrentNeeds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    FDinosaurMemory Memory;
    
    // === DOMESTICAÇÃO ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domestication")
    EDomesticationLevel DomesticationLevel = EDomesticationLevel::Wild;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated = false;
    
    // === ROTINAS DIÁRIAS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routines")
    TArray<FVector> PatrolPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routines")
    float RoutineVariation = 0.2f; // Quanto a rotina pode variar (0.0 = rígida, 1.0 = totalmente aleatória)
    
    // === FUNÇÕES PÚBLICAS ===
    
    // Gestão de estado comportamental
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }
    
    // Gestão de necessidades
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Needs")
    float GetMostUrgentNeed() const;
    
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SatisfyNeed(const FString& NeedType, float Amount);
    
    // Gestão de memória
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberLocation(FVector Location, const FString& LocationType);
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor, float ThreatLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetNearestKnownLocation(const FString& LocationType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasSeenActorRecently(AActor* Actor, float TimeThreshold = 300.0f) const;
    
    // Domesticação
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessPlayerInteraction(bool bPositive, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void UpdateDomesticationLevel();
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanAcceptFood() const;
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool WillFollowPlayer() const;
    
    // Utilidades para Behavior Trees
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    void UpdateBlackboardValues();
    
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    FVector GetBestLocationForCurrentNeed() const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    bool ShouldFleeFromActor(AActor* Actor) const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    bool ShouldInvestigateActor(AActor* Actor) const;

private:
    // === FUNÇÕES INTERNAS ===
    void InitializePersonality();
    void InitializeSpeciesDefaults();
    void ProcessDailyRoutine();
    void DecayNeeds(float DeltaTime);
    void UpdateMemoryDecay(float DeltaTime);
    
    // === VARIÁVEIS INTERNAS ===
    float LastRoutineUpdate = 0.0f;
    float LastNeedsUpdate = 0.0f;
    float LastMemoryUpdate = 0.0f;
    
    // Timers para diferentes sistemas
    const float RoutineUpdateInterval = 30.0f; // Actualizar rotina a cada 30 segundos
    const float NeedsUpdateInterval = 60.0f;   // Actualizar necessidades a cada minuto
    const float MemoryUpdateInterval = 120.0f; // Limpar memória a cada 2 minutos
};