#include "NPCBehaviorSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 vezes por segundo para performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar personalidade única
    InitializePersonality();
    
    // Configurar território baseado na localização inicial
    if (TerritoryCenter.IsZero())
    {
        TerritoryCenter = GetOwner()->GetActorLocation();
    }
    
    // Encontrar o Time Manager
    TimeManager = Cast<ATimeOfDayManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATimeOfDayManager::StaticClass()));
    
    // Configurar rotinas padrão baseadas na espécie
    if (DailyRoutines.Num() == 0)
    {
        SetupDefaultRoutines();
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update necessidades básicas
    UpdateNeeds(DeltaTime);
    
    // Processar rotina diária
    ProcessDailyRoutine();
    
    // Atualizar comportamento baseado nas necessidades
    UpdateBehaviorBasedOnNeeds();
    
    // Processar interações sociais
    ProcessSocialInteractions();
    
    // Atualizar memória
    UpdateMemory(DeltaTime);
}

void UNPCBehaviorComponent::InitializePersonality()
{
    // Gerar personalidade baseada na espécie com variação individual
    switch (Species)
    {
        case EDinosaurSpecies::TyrannosaurusRex:
            Personality.Aggressiveness = FMath::RandRange(0.8f, 1.0f);
            Personality.Fearfulness = FMath::RandRange(0.0f, 0.2f);
            Personality.Territoriality = FMath::RandRange(0.7f, 1.0f);
            Personality.Intelligence = FMath::RandRange(0.6f, 0.8f);
            Personality.Sociability = FMath::RandRange(0.0f, 0.3f);
            break;
            
        case EDinosaurSpecies::Velociraptor:
            Personality.Aggressiveness = FMath::RandRange(0.6f, 0.9f);
            Personality.Intelligence = FMath::RandRange(0.8f, 1.0f);
            Personality.Sociability = FMath::RandRange(0.7f, 1.0f);
            Personality.Curiosity = FMath::RandRange(0.6f, 0.9f);
            Personality.Fearfulness = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case EDinosaurSpecies::Triceratops:
            Personality.Aggressiveness = FMath::RandRange(0.3f, 0.7f);
            Personality.Fearfulness = FMath::RandRange(0.2f, 0.5f);
            Personality.Territoriality = FMath::RandRange(0.4f, 0.8f);
            Personality.Sociability = FMath::RandRange(0.5f, 0.8f);
            break;
            
        case EDinosaurSpecies::Compsognathus:
            Personality.Fearfulness = FMath::RandRange(0.7f, 1.0f);
            Personality.Curiosity = FMath::RandRange(0.6f, 0.9f);
            Personality.Sociability = FMath::RandRange(0.8f, 1.0f);
            Personality.Aggressiveness = FMath::RandRange(0.1f, 0.4f);
            Personality.Intelligence = FMath::RandRange(0.5f, 0.7f);
            break;
            
        case EDinosaurSpecies::Parasaurolophus:
            Personality.Sociability = FMath::RandRange(0.8f, 1.0f);
            Personality.Fearfulness = FMath::RandRange(0.6f, 0.9f);
            Personality.Curiosity = FMath::RandRange(0.4f, 0.7f);
            Personality.Aggressiveness = FMath::RandRange(0.0f, 0.3f);
            break;
            
        default:
            // Valores médios para outras espécies
            Personality.Aggressiveness = FMath::RandRange(0.3f, 0.7f);
            Personality.Fearfulness = FMath::RandRange(0.3f, 0.7f);
            Personality.Curiosity = FMath::RandRange(0.3f, 0.7f);
            Personality.Sociability = FMath::RandRange(0.3f, 0.7f);
            Personality.Territoriality = FMath::RandRange(0.3f, 0.7f);
            Personality.Intelligence = FMath::RandRange(0.3f, 0.7f);
            break;
    }
}

void UNPCBehaviorComponent::SetupDefaultRoutines()
{
    DailyRoutines.Empty();
    
    // Rotinas baseadas na espécie
    if (IsHerbivore())
    {
        // Herbívoros: foraging de manhã, descanso ao meio-dia, foraging à tarde
        FDailyRoutine MorningForaging;
        MorningForaging.StartTime = 6.0f;
        MorningForaging.EndTime = 10.0f;
        MorningForaging.Activity = EDinosaurBehaviorState::Foraging;
        MorningForaging.Priority = 1.0f;
        DailyRoutines.Add(MorningForaging);
        
        FDailyRoutine MidDayRest;
        MidDayRest.StartTime = 12.0f;
        MidDayRest.EndTime = 14.0f;
        MidDayRest.Activity = EDinosaurBehaviorState::Resting;
        MidDayRest.Priority = 0.8f;
        DailyRoutines.Add(MidDayRest);
        
        FDailyRoutine EveningForaging;
        EveningForaging.StartTime = 16.0f;
        EveningForaging.EndTime = 19.0f;
        EveningForaging.Activity = EDinosaurBehaviorState::Foraging;
        EveningForaging.Priority = 1.0f;
        DailyRoutines.Add(EveningForaging);
    }
    else
    {
        // Carnívoros: hunting patterns baseados na espécie
        FDailyRoutine DawnHunt;
        DawnHunt.StartTime = 5.0f;
        DawnHunt.EndTime = 8.0f;
        DawnHunt.Activity = EDinosaurBehaviorState::Hunting;
        DawnHunt.Priority = 1.0f;
        DailyRoutines.Add(DawnHunt);
        
        FDailyRoutine DuskHunt;
        DuskHunt.StartTime = 18.0f;
        DuskHunt.EndTime = 21.0f;
        DuskHunt.Activity = EDinosaurBehaviorState::Hunting;
        DuskHunt.Priority = 1.0f;
        DailyRoutines.Add(DuskHunt);
    }
    
    // Todas as espécies bebem água
    FDailyRoutine MorningDrink;
    MorningDrink.StartTime = 7.0f;
    MorningDrink.EndTime = 8.0f;
    MorningDrink.Activity = EDinosaurBehaviorState::Drinking;
    MorningDrink.Priority = 0.9f;
    DailyRoutines.Add(MorningDrink);
    
    FDailyRoutine EveningDrink;
    EveningDrink.StartTime = 17.0f;
    EveningDrink.EndTime = 18.0f;
    EveningDrink.Activity = EDinosaurBehaviorState::Drinking;
    EveningDrink.Priority = 0.9f;
    DailyRoutines.Add(EveningDrink);
}

bool UNPCBehaviorComponent::IsHerbivore() const
{
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Pachycephalosaurus:
        case EDinosaurSpecies::Triceratops:
        case EDinosaurSpecies::Brachiosaurus:
        case EDinosaurSpecies::Ankylosaurus:
            return true;
        default:
            return false;
    }
}

void UNPCBehaviorComponent::ProcessDailyRoutine()
{
    if (!TimeManager)
        return;
        
    float CurrentTime = TimeManager->GetCurrentTimeOfDay();
    
    // Encontrar a actividade apropriada para a hora atual
    FDailyRoutine* BestRoutine = nullptr;
    float BestPriority = 0.0f;
    
    for (FDailyRoutine& Routine : DailyRoutines)
    {
        bool IsInTimeRange = false;
        
        if (Routine.StartTime <= Routine.EndTime)
        {
            // Mesmo dia
            IsInTimeRange = (CurrentTime >= Routine.StartTime && CurrentTime <= Routine.EndTime);
        }
        else
        {
            // Atravessa meia-noite
            IsInTimeRange = (CurrentTime >= Routine.StartTime || CurrentTime <= Routine.EndTime);
        }
        
        if (IsInTimeRange && Routine.Priority > BestPriority)
        {
            BestRoutine = &Routine;
            BestPriority = Routine.Priority;
        }
    }
    
    // Aplicar a rotina se encontrada e diferente do estado atual
    if (BestRoutine && CurrentBehaviorState != BestRoutine->Activity)
    {
        // Verificar se as necessidades permitem esta actividade
        if (CanPerformActivity(BestRoutine->Activity))
        {
            SetBehaviorState(BestRoutine->Activity);
        }
    }
}

bool UNPCBehaviorComponent::CanPerformActivity(EDinosaurBehaviorState Activity) const
{
    switch (Activity)
    {
        case EDinosaurBehaviorState::Foraging:
        case EDinosaurBehaviorState::Hunting:
            return Energy > 0.2f; // Precisa de energia para procurar comida
            
        case EDinosaurBehaviorState::Drinking:
            return Energy > 0.1f; // Menos energia necessária para beber
            
        case EDinosaurBehaviorState::Resting:
            return true; // Sempre pode descansar
            
        case EDinosaurBehaviorState::Socializing:
            return Energy > 0.3f && Health > 0.5f;
            
        default:
            return true;
    }
}

void UNPCBehaviorComponent::UpdateNeeds(float DeltaTime)
{
    LastNeedsUpdate += DeltaTime;
    if (LastNeedsUpdate < 1.0f) // Update a cada segundo
        return;
        
    float TimeMultiplier = LastNeedsUpdate;
    LastNeedsUpdate = 0.0f;
    
    // Degradação natural das necessidades
    float BaseDecayRate = 0.001f; // 0.1% por segundo
    
    // Fome aumenta mais rápido para carnívoros
    float HungerDecayRate = IsHerbivore() ? BaseDecayRate : BaseDecayRate * 1.5f;
    Hunger = FMath::Clamp(Hunger - (HungerDecayRate * TimeMultiplier), 0.0f, 1.0f);
    
    // Sede aumenta para todos
    Thirst = FMath::Clamp(Thirst - (BaseDecayRate * 1.2f * TimeMultiplier), 0.0f, 1.0f);
    
    // Energia diminui com actividade
    float EnergyDecayRate = BaseDecayRate;
    if (CurrentBehaviorState == EDinosaurBehaviorState::Hunting || 
        CurrentBehaviorState == EDinosaurBehaviorState::Fleeing)
    {
        EnergyDecayRate *= 3.0f; // Actividades intensas consomem mais energia
    }
    else if (CurrentBehaviorState == EDinosaurBehaviorState::Resting)
    {
        EnergyDecayRate *= -2.0f; // Descansar restaura energia
    }
    
    Energy = FMath::Clamp(Energy - (EnergyDecayRate * TimeMultiplier), 0.0f, 1.0f);
    
    // Saúde regenera lentamente quando descansando e bem alimentado
    if (CurrentBehaviorState == EDinosaurBehaviorState::Resting && 
        Hunger > 0.7f && Thirst > 0.7f)
    {
        Health = FMath::Clamp(Health + (BaseDecayRate * 0.5f * TimeMultiplier), 0.0f, 1.0f);
    }
}

void UNPCBehaviorComponent::UpdateBehaviorBasedOnNeeds()
{
    // Prioridades de sobrevivência sobrepõem rotinas
    if (Health < 0.3f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Resting);
        return;
    }
    
    if (Thirst < 0.2f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Drinking);
        return;
    }
    
    if (Hunger < 0.2f)
    {
        if (IsHerbivore())
        {
            SetBehaviorState(EDinosaurBehaviorState::Foraging);
        }
        else
        {
            SetBehaviorState(EDinosaurBehaviorState::Hunting);
        }
        return;
    }
    
    if (Energy < 0.1f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Resting);
        return;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
        return;
        
    EDinosaurBehaviorState PreviousState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    
    // Notificar outros sistemas da mudança de comportamento
    OnBehaviorStateChanged(PreviousState, NewState);
}

void UNPCBehaviorComponent::OnBehaviorStateChanged(EDinosaurBehaviorState PreviousState, EDinosaurBehaviorState NewState)
{
    // Atualizar Blackboard se existir
    if (AAIController* AIController = Cast<AAIController>(Cast<APawn>(GetOwner())->GetController()))
    {
        if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
    }
    
    // Log para debugging
    UE_LOG(LogTemp, Log, TEXT("%s changed behavior from %s to %s"), 
           *GetOwner()->GetName(),
           *UEnum::GetValueAsString(PreviousState),
           *UEnum::GetValueAsString(NewState));
}

bool UNPCBehaviorComponent::CanBeDomesticated() const
{
    // Apenas herbívoros pequenos podem ser domesticados
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Pachycephalosaurus:
            return true;
        default:
            return false;
    }
}

void UNPCBehaviorComponent::ProcessDomesticationInteraction(float PositiveInteraction)
{
    if (!CanBeDomesticated())
        return;
        
    // Factores que influenciam a domesticação
    float PersonalityFactor = (1.0f - Personality.Fearfulness) * 0.5f + Personality.Curiosity * 0.3f + Personality.Intelligence * 0.2f;
    float TrustGain = PositiveInteraction * PersonalityFactor * 0.1f;
    
    Memory.PlayerTrustLevel = FMath::Clamp(Memory.PlayerTrustLevel + TrustGain, -1.0f, 1.0f);
    
    // Atualizar nível de domesticação baseado na confiança
    if (Memory.PlayerTrustLevel >= 0.8f)
        DomesticationLevel = EDomesticationLevel::Loyal;
    else if (Memory.PlayerTrustLevel >= 0.6f)
        DomesticationLevel = EDomesticationLevel::Bonded;
    else if (Memory.PlayerTrustLevel >= 0.4f)
        DomesticationLevel = EDomesticationLevel::Friendly;
    else if (Memory.PlayerTrustLevel >= 0.2f)
        DomesticationLevel = EDomesticationLevel::Tolerant;
    else if (Memory.PlayerTrustLevel >= 0.0f)
        DomesticationLevel = EDomesticationLevel::Curious;
    else if (Memory.PlayerTrustLevel >= -0.3f)
        DomesticationLevel = EDomesticationLevel::Wary;
    else
        DomesticationLevel = EDomesticationLevel::Wild;
}