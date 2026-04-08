#include "NPCBehaviorSystem.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 100ms para performance

    // Valores padrão para características individuais
    Aggressiveness = FMath::RandRange(0.1f, 0.9f);
    Curiosity = FMath::RandRange(0.2f, 0.8f);
    Sociability = FMath::RandRange(0.3f, 0.7f);
    Intelligence = FMath::RandRange(0.4f, 0.9f);
    Territoriality = FMath::RandRange(0.2f, 0.8f);

    CurrentEmotionalState = EDinosaurEmotionalState::Calm;
    CurrentTimeOfDay = 0.0f;
    DomesticationProgress = 0.0f;
    bIsBeingDomesticated = false;
    
    LastRoutineUpdate = 0.0f;
    LastEmotionalUpdate = 0.0f;
    LastMemoryUpdate = 0.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Gerar rotina diária única para este dinossauro
    DailyRoutine.WakeUpTime = FMath::RandRange(0.2f, 0.3f); // Entre 4:48 e 7:12
    DailyRoutine.FeedingTime1 = DailyRoutine.WakeUpTime + FMath::RandRange(0.05f, 0.1f);
    DailyRoutine.FeedingTime2 = FMath::RandRange(0.6f, 0.75f); // Entre 14:24 e 18:00
    DailyRoutine.RestTime = FMath::RandRange(0.45f, 0.55f); // Entre 10:48 e 13:12
    DailyRoutine.SleepTime = FMath::RandRange(0.8f, 0.9f); // Entre 19:12 e 21:36

    // Definir território inicial
    if (AActor* Owner = GetOwner())
    {
        DailyRoutine.TerritoryCenter = Owner->GetActorLocation();
        DailyRoutine.TerritoryRadius = FMath::RandRange(500.0f, 2000.0f) * Territoriality;
        
        // Áreas preferenciais dentro do território
        FVector RandomOffset1 = FMath::VRand() * DailyRoutine.TerritoryRadius * 0.6f;
        FVector RandomOffset2 = FMath::VRand() * DailyRoutine.TerritoryRadius * 0.4f;
        
        DailyRoutine.PreferredFeedingArea = DailyRoutine.TerritoryCenter + RandomOffset1;
        DailyRoutine.PreferredRestingArea = DailyRoutine.TerritoryCenter + RandomOffset2;
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Atualizar tempo do dia (ciclo de 24 minutos = 1 dia no jogo)
    CurrentTimeOfDay = FMath::Fmod(CurrentTime / 1440.0f, 1.0f); // 1440 segundos = 24 minutos

    // Atualizar rotina diária a cada 5 segundos
    if (CurrentTime - LastRoutineUpdate > 5.0f)
    {
        ProcessDailyRoutine();
        LastRoutineUpdate = CurrentTime;
    }

    // Atualizar estado emocional a cada 2 segundos
    if (CurrentTime - LastEmotionalUpdate > 2.0f)
    {
        UpdateEmotionalState();
        LastEmotionalUpdate = CurrentTime;
    }

    // Processar domesticação se aplicável
    if (bIsBeingDomesticated)
    {
        ProcessDomestication(DeltaTime);
    }

    // Verificar proximidade do jogador
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
        if (DistanceToPlayer < 3000.0f) // Dentro de 30 metros
        {
            ReactToPlayer(Player, DistanceToPlayer);
        }
    }
}

void UNPCBehaviorComponent::UpdateEmotionalState()
{
    // Lógica para mudança de estado emocional baseada em fatores externos
    
    // Verificar se está na hora de comer
    float TimeSinceLastMeal = FMath::Abs(CurrentTimeOfDay - DailyRoutine.FeedingTime1);
    float TimeSinceSecondMeal = FMath::Abs(CurrentTimeOfDay - DailyRoutine.FeedingTime2);
    
    if (FMath::Min(TimeSinceLastMeal, TimeSinceSecondMeal) < 0.02f) // Dentro de ~30 minutos do jogo
    {
        if (CurrentEmotionalState == EDinosaurEmotionalState::Calm)
        {
            CurrentEmotionalState = EDinosaurEmotionalState::Hungry;
        }
    }

    // Verificar se está na hora de dormir
    if (FMath::Abs(CurrentTimeOfDay - DailyRoutine.SleepTime) < 0.05f)
    {
        if (CurrentEmotionalState != EDinosaurEmotionalState::Fearful && 
            CurrentEmotionalState != EDinosaurEmotionalState::Aggressive)
        {
            CurrentEmotionalState = EDinosaurEmotionalState::Tired;
        }
    }

    // Retornar ao estado calmo gradualmente
    if (CurrentEmotionalState == EDinosaurEmotionalState::Alert)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% de chance por update
        {
            CurrentEmotionalState = EDinosaurEmotionalState::Calm;
        }
    }
}

void UNPCBehaviorComponent::ProcessDailyRoutine()
{
    // Determinar ação atual baseada na hora do dia e personalidade
    
    if (CurrentTimeOfDay < DailyRoutine.WakeUpTime || CurrentTimeOfDay > DailyRoutine.SleepTime)
    {
        // Período de sono - comportamento mínimo
        if (CurrentEmotionalState == EDinosaurEmotionalState::Calm)
        {
            CurrentEmotionalState = EDinosaurEmotionalState::Tired;
        }
    }
    else if (FMath::Abs(CurrentTimeOfDay - DailyRoutine.FeedingTime1) < 0.05f ||
             FMath::Abs(CurrentTimeOfDay - DailyRoutine.FeedingTime2) < 0.05f)
    {
        // Hora de comer
        CurrentEmotionalState = EDinosaurEmotionalState::Hungry;
    }
    else if (FMath::Abs(CurrentTimeOfDay - DailyRoutine.RestTime) < 0.05f)
    {
        // Hora de descansar
        if (CurrentEmotionalState == EDinosaurEmotionalState::Calm)
        {
            CurrentEmotionalState = EDinosaurEmotionalState::Tired;
        }
    }
}

void UNPCBehaviorComponent::ReactToPlayer(AActor* Player, float Distance)
{
    if (!Player) return;

    // Atualizar memória sobre o jogador
    Memory.LastSeenPlayerLocation = Player->GetActorLocation();
    Memory.LastPlayerEncounterTime = GetWorld()->GetTimeSeconds();
    Memory.HasSeenPlayer = true;

    // Reação baseada no tipo de comportamento e distância
    switch (BehaviorType)
    {
        case EDinosaurBehaviorType::Herbivore_Passive:
            if (Distance < 500.0f) // 5 metros
            {
                if (Memory.TrustLevelTowardsPlayer < 0.3f)
                {
                    CurrentEmotionalState = EDinosaurEmotionalState::Fearful;
                }
                else if (Memory.TrustLevelTowardsPlayer > 0.7f)
                {
                    CurrentEmotionalState = EDinosaurEmotionalState::Curious;
                }
                else
                {
                    CurrentEmotionalState = EDinosaurEmotionalState::Alert;
                }
            }
            break;

        case EDinosaurBehaviorType::Carnivore_Ambush:
            if (Distance < 1000.0f && Distance > 300.0f) // Zona de emboscada
            {
                CurrentEmotionalState = EDinosaurEmotionalState::Alert;
                if (Aggressiveness > 0.6f)
                {
                    CurrentEmotionalState = EDinosaurEmotionalState::Aggressive;
                }
            }
            break;

        case EDinosaurBehaviorType::Carnivore_Apex:
            if (Distance < 1500.0f)
            {
                CurrentEmotionalState = EDinosaurEmotionalState::Territorial;
                if (Distance < 800.0f)
                {
                    CurrentEmotionalState = EDinosaurEmotionalState::Aggressive;
                }
            }
            break;

        default:
            if (Distance < 800.0f)
            {
                CurrentEmotionalState = EDinosaurEmotionalState::Alert;
            }
            break;
    }
}

void UNPCBehaviorComponent::UpdateMemory(AActor* Actor, bool IsThreat)
{
    if (!Actor) return;

    if (IsThreat)
    {
        Memory.KnownThreats.AddUnique(Actor);
        Memory.KnownDangerZones.AddUnique(Actor->GetActorLocation());
    }
    else
    {
        Memory.KnownAllies.AddUnique(Actor);
    }
}

bool UNPCBehaviorComponent::ShouldEnterDomesticationProcess()
{
    // Apenas herbívoros passivos podem ser domesticados
    if (BehaviorType != EDinosaurBehaviorType::Herbivore_Passive)
    {
        return false;
    }

    // Deve ter visto o jogador múltiplas vezes
    if (!Memory.HasSeenPlayer)
    {
        return false;
    }

    // Jogador deve estar próximo por tempo suficiente
    float TimeSinceLastSeen = GetWorld()->GetTimeSeconds() - Memory.LastPlayerEncounterTime;
    if (TimeSinceLastSeen < 30.0f && Memory.TrustLevelTowardsPlayer < 0.9f)
    {
        return true;
    }

    return false;
}

void UNPCBehaviorComponent::ProcessDomestication(float DeltaTime)
{
    if (!ShouldEnterDomesticationProcess())
    {
        bIsBeingDomesticated = false;
        return;
    }

    // Processo muito lento de domesticação
    float DomesticationRate = 0.001f; // Base rate
    
    // Fatores que influenciam a velocidade
    if (CurrentEmotionalState == EDinosaurEmotionalState::Calm)
    {
        DomesticationRate *= 2.0f;
    }
    else if (CurrentEmotionalState == EDinosaurEmotionalState::Fearful)
    {
        DomesticationRate *= 0.1f; // Muito mais lento se com medo
    }

    // Personalidade influencia
    DomesticationRate *= (1.0f - Aggressiveness); // Menos agressivo = mais fácil
    DomesticationRate *= (Curiosity + 0.5f); // Mais curioso = mais fácil
    DomesticationRate *= (Intelligence + 0.5f); // Mais inteligente = mais fácil

    Memory.TrustLevelTowardsPlayer += DomesticationRate * DeltaTime;
    Memory.TrustLevelTowardsPlayer = FMath::Clamp(Memory.TrustLevelTowardsPlayer, 0.0f, 1.0f);

    // Domesticação completa
    if (Memory.TrustLevelTowardsPlayer >= 0.95f)
    {
        bIsBeingDomesticated = false;
        // Aqui poderia disparar evento de domesticação completa
    }
}

FVector UNPCBehaviorComponent::GetCurrentGoalLocation()
{
    switch (CurrentEmotionalState)
    {
        case EDinosaurEmotionalState::Hungry:
            return DailyRoutine.PreferredFeedingArea;
            
        case EDinosaurEmotionalState::Tired:
            return DailyRoutine.PreferredRestingArea;
            
        case EDinosaurEmotionalState::Fearful:
            // Fugir para área segura conhecida ou borda do território
            return DailyRoutine.TerritoryCenter + (FMath::VRand() * DailyRoutine.TerritoryRadius);
            
        case EDinosaurEmotionalState::Territorial:
            return DailyRoutine.TerritoryCenter;
            
        default:
            // Patrulhar território aleatoriamente
            FVector RandomPoint = DailyRoutine.TerritoryCenter + 
                                (FMath::VRand() * DailyRoutine.TerritoryRadius * 0.7f);
            return RandomPoint;
    }
}

bool UNPCBehaviorComponent::IsInTerritory(FVector Location)
{
    float DistanceFromCenter = FVector::Dist(Location, DailyRoutine.TerritoryCenter);
    return DistanceFromCenter <= DailyRoutine.TerritoryRadius;
}